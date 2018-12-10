
#include <simgrid/s4u.hpp>//biblioteca do SimGrid
#include <string>//biblioteca para usar funcoes de string
#include <iostream>//biblioteca para entrada e saida padrão do C++
#include <fstream>//biblioteca do C++ para eitura de arquivos
#include <vector>

//cria um canal para obter informações desse exemplo
XBT_LOG_NEW_DEFAULT_CATEGORY(main, "O canal do XBT para este projeto");

using namespace simgrid::s4u;//facilita a escrita do codigo, ja que quase
                            //todas as funções usam esse namespace

using namespace std;//idem anterior

//define usado para tratar o vetor como uma matriz.
#define MA(A,i,j,tam_col) A[i*tam_col + j]

//um vetor que ira representar onde esta os objetos. a coluna 0 é o objeto e a coluna 1 é o seu servidor.
int *objetos;
int tam;

static void middleServer(vector<string> args);
static void middleServerReception(vector<string> args);
static void reception(vector<string> args);
static void answer(vector<string> args);
static void request(vector<string> args);

//utilizado somente para inicar as funções da simulação.
int main(int argc, char** argv)
{
  if(argc != 3)
  {
    cout << "O programa espera um arquivo de plataforma e um arquivo de deployment" << endl;
    exit(1);
  }

  ifstream obj("arquivos/objetos.txt");

  int bufer;
  char aux;
  int lin = 0;

  //descobre quantos objetos tem.
  while(obj >> aux)
  {
    lin++;
  }
  lin /= 2;
  obj.clear();
  obj.seekg(0);
  tam = lin;//tam é uma variavel global para saber a quantidade de objetos.
  objetos = new int[lin*2];
  int j = 0;

  //le os objetos e seus servidores.
  while(obj >> bufer)
  {
    objetos[j] = bufer;
    j++;
  }

  Engine e(&argc,argv);//cria a base da simulacao

  e.register_function("request", &request);//registra a função para ser utilizada na simulação
  e.register_function("answer", &answer);
  e.register_function("reception", &reception);
  e.register_function("middleserver",&middleServer);

  e.load_platform(argv[1]);//carrega a plataforma usada
  e.load_deployment(argv[2]);//le o arquivo xml com os atores

  e.run();//roda a simulacao
  return 0;
}
//funciona como a resposta do servidor central para o cliente
static void middleServerReception(vector<string> args)
{
  MailboxPtr mailbox = Mailbox::by_name(args[0]);//cria uma mailbox que
                                                //serve para receber e mandar a mensagem.
  XBT_INFO("entrei no servidor central para enviar para %s",args[2].c_str());

  //string usada para receber a mensagem da mailbox
  string *msg;
  msg = static_cast<string*>(mailbox->get());

  args[0] = string("Mailbox-") + args[2];

  XBT_INFO("O objeto requerido foi: %s.",msg->c_str());

  //cria um vector de strings (template) e preenche com as informações nescessarias para cirar os atores do servidor
  vector<string> str1;
  str1.push_back(args[0]);//Passa o nome da mailbox para args[0]
  str1.push_back(args[1]);//passa o nome do servidor para args[1]
  str1.push_back(args[2]);//passa o nome do cliente para args[2]


  mailbox = Mailbox::by_name(args[0]);

  XBT_INFO("enviando o objeto %s para o %s",msg->c_str(),args[2].c_str());
  //cria um actor para o envio do objeto.
  mailbox->set_receiver(Actor::create(args[2],Host::by_name(args[2]),reception,str1));

  //envio dos objetos.
  mailbox->put(new string(*msg),1);

  str1.clear();//limpa o vector
  this_actor::exit();//mata o ator
}

//funciona como o servidor central, que direciona para os demais servidores.
static void middleServer(vector<string> args)
{
  MailboxPtr mailbox = Mailbox::by_name(args[0]);//cria uma mailbox que
                                                //serve para receber e mandar a mensagem.
  XBT_INFO("entrei no servidor central");


  //string usada para receber a mensagem da mailbox
  string *msg;
  msg = static_cast<string*>(mailbox->get());
  int objeto_requerido = strtol(msg->c_str(),NULL,10);
  string name;

  XBT_INFO("O objeto requerido foi: %d.",objeto_requerido);

  for (int i = 0; i < tam; i++) {
    if(MA(objetos,i,0,2) == objeto_requerido)
    {
      name = string("server") + to_string( MA(objetos,i,1,2) );
      break;
    }
  }

  //atualiza a mailbox, para enviar para o servidor certo.
  args[0] = string("Mailbox-")+name;
  mailbox = Mailbox::by_name(args[0]);

  //cria um vector de strings (template) e preenche com as informações nescessarias para cirar os atores do servidor
  vector<string> str1;
  str1.push_back(args[0]);//Passa o nome da mailbox para args[0]
  str1.push_back(name);//passa o nome do servidor para args[1]
  str1.push_back(args[1]);//passa o nome do cliente para args[2]

  XBT_INFO("enviando a requisição do objeto %d para o servidor %s vinda de %s",objeto_requerido,name.c_str(),args[1].c_str());

  //cria um actor para o envio do objeto.
  mailbox->set_receiver(Actor::create(name,Host::by_name(name),answer,str1));

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
  XBT_INFO("entrei no servidor %s e enviarei o objeto para o servidor central",args[1].c_str());


  //cria um vector de strings (template) e preenche com as informações nescessarias para cirar os atores do servidor
  vector<string> str1;

  //string usada para receber a mensagem da mailbox
  string *msg;
  msg = static_cast<string*>(mailbox->get());

  XBT_INFO("O objeto recebida foi: %s.",msg->c_str());

  str1.push_back(args[0]);//passa o nome da mailbox para args[0]
  str1.push_back(args[1]);//passa o nome do servidor para args[1]
  str1.push_back(args[2]);//passa o nome do cliente para args[2]

  //cria um actor para o envio do objeto.
  mailbox->set_receiver(Actor::create("middleServer",Host::by_name("middleserver"),middleServerReception,str1));

  //envio dos objetos.
  mailbox->put(new string("o objeto"),1);

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
  mailbox->set_receiver(Actor::create("middle",Host::by_name("middleserver"),middleServer, str1));

  //envio um pacote com x bytes para requisitar um objeto.
  mailbox->put(new string(args[1]),1);

  str1.clear();//limpa o vector
  this_actor::exit();//mata o ator
}
