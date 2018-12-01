#include <simgrid/s4u.hpp>//biblioteca do SimGrid
#include <string>//biblioteca para usar funcoes de string
#include <iostream>

//cria um canal para obter informações desse exemplo
XBT_LOG_NEW_DEFAULT_CATEGORY(main_primeiro_teste, "O canal do XBT para este projeto");

using namespace simgrid::s4u;//facilita a escrita do codigo, ja que quase
                            //todas as funções usam esse namespace
using namespace std;

static void reception(vector<string> args)
{
  XBT_INFO("entrei no cliente %s e estou recebendo o objeto do servidor %s",args[2].c_str(),args[1].c_str());
  //cria uma mailbox para comunicar entre o servidor e o cliente, servindo de recepção do objeto.
  MailboxPtr mailbox = simgrid::s4u::Mailbox::by_name(args[0]);

  //pega um dado da mailbox, precisa de uma função put sendo executada em simutaneo.
  string *msg = static_cast<string*>(mailbox->get());

  //pega o tempo atual da simulação
  double *aux = new double;
  *aux = Engine::get_clock();

  XBT_INFO("recebi o objeto de %s no tempo %lf. A mensagem era: %s",args[1].c_str(),*aux - stod(*msg),msg->c_str());

  this_actor::exit();
}

static void answer(vector<string> args)
{
  MailboxPtr mailbox = Mailbox::by_name(args[0]);//cria uma mailbox que
                                                //serve para receber e mandar a mensagem.
  XBT_INFO("entrei no servidor %s e enviarei o objeto para o cliente %s",args[1].c_str(),args[2].c_str());

  string *msg;
  //cria um vector de strings (template) e preenche com as informações nescessarias para cirar os atores do servidor
  vector<string> str1;

  msg = static_cast<string*>(mailbox->get());

  XBT_INFO("a mensagem recebida foi: %s.",msg->c_str());

  //pega o tempo atual, ou seja, do momento que a mensagem começou a ser enviada.
  double *tempo = new double;
  *tempo = Engine::get_clock();

  str1.push_back(args[0]);//Passa o nome da mailbox para args[0]
  str1.push_back(args[1]);//passa o nome do servidor para args[1]
  str1.push_back(args[2]);//passa o nome do cliente para args[2]

  mailbox->set_receiver(Actor::create(args[2],Host::by_name(args[2]),reception,str1));//cria um actor para o envio do objeto.

  mailbox->put(new string(to_string(*tempo)),50*1024*1024);//envio dos objetos.

  //mailbox->put(new string("objeto recebido"),2e10);

  XBT_INFO("objeto enviado");
  str1.clear();
  this_actor::exit();
}

static void request(vector<string> args)
{
  XBT_INFO("entrei na request vindo do cliente %s para o servidor %s.",this_actor::get_host()->get_cname(),args[1].c_str());
  //cria o nome da minha Mailbox
  string name = string("Mailbox-")+args[1];//+string(" to ")+this_actor::get_host()->get_name();

  //cria uma mailbox para comunicar entre o cliente e o servido, servindo de mensagem de get.
  MailboxPtr mailbox = simgrid::s4u::Mailbox::by_name(name);

  //cria um vector de strings (template) e preenche com as informações nescessarias para cirar os atores do servidor
  vector<string> str1;
  str1.push_back(name);//Passa o nome da mailbox para args[0]
  str1.push_back(args[1]);//passa o nome do servidor para args[1]
  str1.push_back(this_actor::get_host()->get_name());//passa o nome do cliente para args[2]

  mailbox->set_receiver(Actor::create(args[1],Host::by_name(args[1]),answer, str1));//cria um actor para o servido que realizara o get.
  mailbox->put(new string(name),30*1024);//envio um pacote com 30 bytes para requisitar um objeto.

  this_actor::exit();
}

int main(int argc, char** argv)
{
  if(argc != 3)
  {
    cout << "O programa espera um arquivo de plataforma e um arquivo de deployment" << endl;
    exit(1);
  }
  //printf("%s e %s\n",argv[1],argv[2]);
  Engine e(&argc,argv);//cria a base da simulacao
  e.register_function("request", &request);//registra a função para ser utilizada na simulação
  e.register_function("answer", &answer);//idem comentario anterior

  e.load_platform(argv[1]);//carrega a plataforma usada
  e.load_deployment(argv[2]);//le o arquivo xml com os atores

  e.run();//roda a simulacao
  return 0;
}
