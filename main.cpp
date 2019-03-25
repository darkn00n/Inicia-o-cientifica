
#include <simgrid/s4u.hpp>//biblioteca do SimGrid
#include <string>//biblioteca para usar funcoes de string
#include <iostream>//biblioteca para entrada e saida padrão do C++
#include <fstream>//biblioteca do C++ para leitura de arquivos
#include <vector>//biblioteca do C++ para controle de vetores

//cria um canal para obter informações desse exemplo
XBT_LOG_NEW_DEFAULT_CATEGORY(main, "O canal do XBT para este projeto");

using namespace simgrid::s4u;//facilita a escrita do codigo, ja que quase
                            //todas as funções usam esse namespace

using namespace std;//idem anterior

//um vetor que ira representar onde esta os objetos.a coluna 0 e o tempo que comeca a valer aquela informacao, a coluna 1 he o objeto e a coluna 2 he o seu servidor.
int* tabela = NULL;
//tamanho da tabela.
int tam_tabela = 0;
//indica o começo da tabela para o tempo atual
int ini_tabela = 0;
//indica o final da tabela para o tempo atual
int fim_tabela = 0;

//o vetor que vai ser utilizado como uma matriz 2x2 para dizer o tamanho do objeto requerido.
int* objetos = NULL;
//tamanho de objetos;
int tam_objetos = 0;

//dois ponteiros que seram usados como vetores para melhor tratamento dos objetos e servidores no intervalo de tempo certo.
int* tamanhos = NULL;//indica quando servidores tem em cada linha do proximo vetor;
int** redirection = NULL;//a primeira posição indica qual o ID do objeto as proximas indicam os servidores que tem esse objeto e a ultima indica qual o proxima a ser usado;
int tam_redirection = 0;//para saber o final dos objetos.

//define usado para tratar o vetor como uma matriz.
#define MA(A,tam_col,i,j) A[(i)*tam_col + j]

static void middleServer(vector<string> args);
static void reception(vector<string> args);
static void answer(vector<string> args);
static void request(vector<string> args);
int* learquivos(string arquivo,int &quant,int col);
void fit_redirection(bool status);
void fit_info_tabela(void);

//utilizado somente para inicar as funções da simulação.
int main(int argc, char** argv)
{
  if(argc != 3)//verfica se os arquivos necessarios para o programa foram inseridos
  {
    cout << "O programa espera um arquivo de plataforma e um arquivo de deployment" << endl;
    exit(1);
  }

  Engine e(&argc,argv);//cria a base da simulacao

  e.register_function("request", &request);//registra a função para ser utilizada na simulação
  e.register_function("answer", &answer);//registra a função para ser utilizada na simulação
  e.register_function("reception", &reception);//registra a função para ser utilizada na simulação
  e.register_function("middleserver",&middleServer);//registra a função para ser utilizada na simulação

  e.load_platform(argv[1]);//carrega a plataforma usada
  e.load_deployment(argv[2]);//le o arquivo xml com os atores

  tabela = learquivos(string("tabela"),tam_tabela,3);//le o arquivo que indica o tempo em que cada objeto fica em qual servidor.
  objetos = learquivos(string("objetos"),tam_objetos,2);//le o arquivo que indica o tamanho de cada objeto

  //atualiza o final da tabela na primeira vez que é necessaria.
  fit_info_tabela();

  fit_redirection(true);//cria a estrutura auxiliar para redirecionamento.

  e.run();//roda a simulacao
  return 0;
}

//funciona como o servidor central, que direciona para os demais servidores.
static void middleServer(vector<string> args)
{
  MailboxPtr mailbox = Mailbox::by_name(args[0]);//cria uma mailbox que
                                                //serve para receber e mandar a mensagem.
  XBT_INFO("entrei no servidor central");

  //string usada para receber a mensagem da mailbox
  string *msg;
  msg = static_cast<string*>(mailbox->get());//recebe a mensagem da mailbox e "casta" para uma string

  int objeto_requerido = strtol(msg->c_str(),NULL,10);//transforma a mensagem em um inteiro(primeiro a string vira um vetor de char e depois um inteiro)

  string name;//string para amarzenar o nome do servido da requisicao

  XBT_INFO("O objeto requerido foi: %d.",objeto_requerido);

  //verifica o momento atual e se é necessario atualizar as tabelas para verificacao dos lugares
  int actual_time = Engine::get_clock();//momento atual dentro da simulacao

  //verifica se o tempo atual já passou do tempo que esta nas instancias atuais e chama a funcao que modifica a redirection
  if(fim_tabela != 0 && fim_tabela != tam_tabela && MA(tabela,3,fim_tabela,0) <= actual_time)
  {
    fit_info_tabela();
    fit_redirection(false);
  }

  int posicao;//serve como auxiliar para a pesquisa de redirecionamento.

  //procura em redirection qual o servidor deve ser enviada a requisicao
  for(int i = 0; i < tam_redirection; i++){
    if(redirection[i][0] == objeto_requerido)//verifico se o ID é igual ao objeto requerido
    {
      posicao = redirection[i][tamanhos[i] + 1];//pega a posicao que tem o servidor a ser usado.
      name = string("server") + to_string( redirection[i][posicao] );//pega o servidor que deve ser usado para a requisicao
      if(posicao == tamanhos[i])//se estiver na ultima posicao volta para a primeira.
      {
          redirection[i][tamanhos[i] + 1] = 1;
      }
      else redirection[i][tamanhos[i] + 1]++;//se nao aumenta para a proxima.

      break;//se achou não he nescessario continuar e sai do loop
    }
  }

  //atualiza a mailbox, para enviar para o servidor certo.
  args[0] = string("Mailbox-")+name;

  //cria um vector de strings (template) e preenche com as informações nescessarias para cirar os atores do servidor
  vector<string> str1;
  str1.push_back(args[0]);//Passa o nome da mailbox para args[0]
  str1.push_back(name);//passa o nome do servidor para args[1]
  str1.push_back(args[1]);//passa o nome do cliente para args[2]

  mailbox = Mailbox::by_name(args[0]);//ajusta a mailbox para a nova que sera utilizada
                                      // entre servidor central e servidor com o objeto

  XBT_INFO("enviando a requisição do objeto %d para o servidor %s vinda de %s",objeto_requerido,name.c_str(),args[1].c_str());

  //cria um actor para o envio do objeto.
  mailbox->set_receiver(Actor::create("answer",Host::by_name(name),answer,str1));

  //envio dos objetos.
  mailbox->put(new string(*msg),1);

  str1.clear();//limpa o vector
  this_actor::exit();//mata o ator
}

//funciona como a chegada do objeto no cliente vinda do servidor central.
static void reception(vector<string> args)
{
  XBT_INFO("entrei no cliente %s e estou recebendo o objeto do servidor %s",args[2].c_str(),args[1].c_str());

  //cria uma mailbox para comunicar entre o servidor e o cliente, servindo de recepção do objeto.
  MailboxPtr mailbox = simgrid::s4u::Mailbox::by_name(args[0]);

  //pega um dado da mailbox, precisa de uma função put sendo executada em simutaneo.
  string *msg = static_cast<string*>(mailbox->get());

  //pega o tempo atual da simulação
  XBT_INFO("recebi o objeto de %s. A mensagem era: %s",args[1].c_str(),msg->c_str());

  this_actor::exit();//mata o ator
}

//função que recebe o pacote, ou seja, funciona como o servido no momento que o objeto está sendo requisitado.
static void answer(vector<string> args)
{
  MailboxPtr mailbox = Mailbox::by_name(args[0]);//cria uma mailbox que
                                                //serve para receber e mandar a mensagem.

  XBT_INFO("entrei no servidor %s e enviarei o objeto para o cliente %s",args[1].c_str(),args[2].c_str());

  //variavel usado para armazenar o tamanho do objeto
  int tamanho_objeto;
  //cria um vector de strings (template) e preenche com as informações nescessarias para criar os atores do servidor
  vector<string> str1;

  //string usada para receber a mensagem da mailbox
  string *msg;
  msg = static_cast<string*>(mailbox->get());//recebe a mensagem da mailbox e "casta" para uma string

  int objeto_requerido = strtol(msg->c_str(),NULL,10);//transforma o id do objeto em inteiro.

  for (int i = 0; i < tam_objetos; i++) {
    if(MA(objetos,2,i,0) == objeto_requerido)
    {
      tamanho_objeto = MA(objetos,2,i,1);
      break;
    }
  }

  XBT_INFO("O objeto requerido foi: %s que tem %d Bytes de tamanho",msg->c_str(),tamanho_objeto);

  str1.push_back(args[0]);//passa o nome da mailbox para args[0]
  str1.push_back(args[1]);//passa o nome do servidor para args[1]
  str1.push_back(args[2]);//passa o nome do cliente para args[2]
  str1.push_back(to_string(tamanho_objeto));//passa o tamanho do objeto para args[3];

  //cria um actor para o envio do objeto.
  mailbox->set_receiver(Actor::create("reception",Host::by_name(args[2]),reception,str1));

  //envio dos objetos.
  mailbox->put(new string("o objeto"),tamanho_objeto);

  str1.clear();//limpa o vector
  this_actor::exit();//mata o ator
}

//funciona como requisição de conteudo do servidor, como um metodo get, envia a requisição para o servidor central.
static void request(vector<string> args)
{
  XBT_INFO("entrei na request vindo do cliente %s para o servidor central.",this_actor::get_host()->get_cname());

  XBT_INFO("dormirei até meu tempo certo, que é %s",args[2].c_str());

  //pega o tempo atual da simulação
  double *aux = new double;
  *aux = Engine::get_clock();

  //o tempo nescessario para que o ator comece a funcionar no tempo recebido no parametro args[2]
  double time_sleep = strtol(args[2].c_str(),NULL,10) - *aux;

  //coloca o ator para dormir até chegar o tempo especificado no xml
  this_actor::sleep_for(time_sleep);

  //cria o nome da minha Mailbox
  string name = string("Mailbox-middle");

  //cria uma mailbox para comunicar entre o cliente e o servido, servindo de mensagem de get.
  MailboxPtr mailbox = Mailbox::by_name(name);

  //cria um vector de strings (template) e preenche com as informações nescessarias para cirar os atores do servidor
  vector<string> str1;
  str1.push_back(name);//Passa o nome da mailbox para args[0]
  str1.push_back(this_actor::get_host()->get_name());//passa o nome do cliente para args[1]

  //cria um actor para o servido central que realizara a resposta.
  mailbox->set_receiver(Actor::create("middleServer",Host::by_name("middleserver"),middleServer, str1));

  //envio um pacote com x bytes para requisitar um objeto.
  mailbox->put(new string(args[1]),1);

  str1.clear();//limpa o vector
  this_actor::exit();//mata o ator
}

//le os tamanhos do objetos e os deixa em memoria, a variavel col deve indicar quantas colunas o arquivo tem.
int* learquivos(string arquivo,int &quant,int col)
{
  int* tamanhos;

  //cria um objeto que representa o arquivo
  ifstream obj("arquivos/"+arquivo+".txt");

  //utilizado para receber um inteiro de cada vez na interacao
  int bufer;

  //numero de linhas do arquivo
  int lin = 0;

  //descobre quantos objetos tem.
  obj >> lin;

  quant = lin;//quant é uma variavel para saber a quantidade de objetos.

  //um vetor que é usado como matriz
  tamanhos  = new int[lin*col];

  int j = 0;

  //le os objetos e seus tamanhos.
  while(obj >> bufer)
  {
    tamanhos[j] = bufer;
    j++;
  }
  obj.close();

  return tamanhos;
}

//atualiza os vetores redirection e tamanhos, status == True indica que é o momento de prencher as estruturas pela primeira vez.
void fit_redirection(bool status)
{
  int* alfabeto = new int[fim_tabela - ini_tabela];//auxilia como um limite superior de todos os possiveis objetos.
  int* quant_alfabeto = new int[fim_tabela - ini_tabela];//vetor para já pegar a quantidade durante o teste de alfabeto
  int ultimo = 0;//variavel para saber o final do alfabeto atual
  bool igual = false;//variavel para saber se o objeto atual esta no alfabeto já lido.
  int posicao = 1;//auxiliar para facilitar a leitura do codigo, utilizada para saber a posicao que sera colocado o servidor na redirection.

  //inicializa com valores pradao os vetores.(utilizo -1 pq esta fora do alfabeto)
  for(size_t i = 0; i < fim_tabela - ini_tabela; i++)
  {
      alfabeto[i] = -1;
      quant_alfabeto[i] = 0;
  }

  //percorro toda a instancia atual da tabela.
  for(size_t i = ini_tabela; i < fim_tabela; i++)
  {
    //para cada posicao da atual instancia da tabela eu percorro a te o final atual do meu vetor alfabeto.
    for (size_t j = 0; j < ultimo; j++)
    {
      //para cada posicao do alfabeto eu verifico se o objeto novo esta dentro do meu alfabeto
      //se estiver eu ativo a flag aumento a quantidade de vezes que aparece e saio.
      if(alfabeto[j] == MA(tabela,3,i,1))
      {
          igual = true;
          quant_alfabeto[j]++;
          break;
      }
    }
    //se a flag estiver como falsa, ou seja, nao tem igual eu adiciono o objeto no alfabeto
    //incremento o final do alfabeto
    //e aumento a quantidade de vezes que o objeto aparece
    if(!igual)
    {
      alfabeto[ultimo] = MA(tabela,3,i,1);
      quant_alfabeto[ultimo]++;
      ultimo++;
    }
    igual = false;//zera a tag para recomecar
  }

  tam_redirection = ultimo;

  //se nao for a primeira vez preciso liberar a memoria
  if(!status)
  {
      free(tamanhos);
      for(size_t i = 0; i < tam_redirection; i++)
      {
        free(redirection[i]);
      }
      free(redirection);
  }

  tamanhos = new int[ultimo];//cria o vetor com a quantidade de objetos.
  redirection = new int*[ultimo];//cria os espacos para armazenar o id do objeto + os servidores que tem ele + o proximo servidor a ser usado.

  //inicializa os vetores e ao mesmo tempo cria as linhas em redirection.
  for(size_t i = 0; i < ultimo; i++)
  {
    tamanhos[i] = quant_alfabeto[i];//guarda na variavel global as quantidades de servidores para cada objeto
    redirection[i] = new int[quant_alfabeto[i] + 2];//cria o espaço para caber o ID + os servidores + proxima posicao a usar
    redirection[i][0] = alfabeto[i];//insere o ID.
    redirection[i][tamanhos[i] + 1] = 1;//inicializa a posicao proximo a usar como o primeiro servidor, é utilizado para preencher os sevidores no proximo loop.
  }

  //percorre toda a tabela na instancia atual
  for(size_t i = ini_tabela; i < fim_tabela; i++)
  {
    //para cada par objeto servidor na instancia eu percorro toda a redirection
    for(size_t j = 0; j < ultimo; j++)
    {
      //verifico se o ID é igual ao objeto na estrutura tabela
      //se for igual eu adiciono o servidor na proxima posicao livre
      //a posicao livre é encontrada na ultima posicao da linha de redirection
      if(MA(tabela,3,i,1) == redirection[j][0])
      {
        posicao = redirection[j][tamanhos[j] + 1];//pega a proxima posicao nao preenchida em redirection[j]
        redirection[j][posicao] = MA(tabela,3,i,2);//preenche a posicao nao inicializada.
        redirection[j][tamanhos[j] + 1]++;//atualiza para manter a proxima posicao atualizada no final de redirection[j]
        break;
      }
    }
  }

  //percorre redirection para concertar o valor do proximo servidor a ser usado.
  for (size_t i = 0; i < ultimo; i++)
  {
    redirection[i][tamanhos[i] + 1] = 1;
  }
}

void fit_info_tabela(void)
{
    int i = 0;
    //a partir do iniciao verifica as posicoes ate o final da tabela
    for(i = fim_tabela;i < tam_tabela - 1;i++)
    {
      //se tiver mudança de tempo define o comeco da nova configuracao de servidor-objeto
      if(MA(tabela,3,i,0) != MA(tabela,3,i+1,0))
      {
        fim_tabela = i+1;//define o fim do momento inicial
        break;
      }
    }
    if(fim_tabela == 0) fim_tabela = tam_tabela;
    if(i == tam_tabela - 1){
        ini_tabela = fim_tabela;
        fim_tabela = tam_tabela;
    }
}
