#include "server.h"
#include<iostream>
#include<csignal>
#include<ctime> //show time

//using namespace std;

server tcp;
pthread_t msg_real[MAX_CLIENT_NO];
int num_message = 0;
int time_send = 2700;

string returnTime()
{
  time_t t = time(0);
  tm *now = localtime(&t);
  string date = to_string(now->tm_hour) + ":" +
                  to_string(now->tm_min) + ":" +
                  to_string(now->tm_sec);
  return date;
}

void* send_client(void* sock)
{
  struct socketDesc *desc = (struct socketDesc *)sock;

  while(1)
  {
    if(!tcp.isOnline() && tcp.GetLastClosed()==desc->id)
    {
      cerr << "close client id:" << desc->id << endl<< "ip:" << desc->ip << endl;
      break;
    }
    string date = returnTime();
    cerr << "server send client date!!!!!!:" << date << endl;
    tcp.Send(date, desc->id);
		sleep(time_send);
		//cerr<<"SEND_CLIENT"<<endl;
  }
  pthread_exit(NULL);
  return 0;
}

void* receive_client(void * sock)
{
  pthread_detach(pthread_self()); //函数尾部自动退出线程
  vector<socketDesc *> desc; //当前收到的msg
  while(1)
  {
    desc = tcp.GetMessage();
    for (int i = 0; i < desc.size(); i++)
    {
      if(desc[i]->message !="")
      {
         if(desc[i]->count==false)
        {
          desc[i]->count = true;
          if(pthread_create(&msg_real[num_message], NULL, send_client, (void *) desc[i])==0)
         {
           cerr << "Thread done" << endl;
          }
        num_message++; //计数来的client个数
        }
        cerr << "id:       " << desc[i]->id << endl
             << "ip:       " << desc[i]->ip << endl
             << "message:  " << desc[i]->message << endl;
        //<< "socketno: " << desc[i]->count << endl;
        tcp.Clean(i);
      }
    }
    usleep(600);
  }
  return 0;
}
/*程序退出时 关闭线程 */
void signal(int use)
{
  tcp.closed();
  exit(1);
}

int main(int argc, char **argv)
{
  if(argc < 2)
  {
    cerr << "参数错误！./server port" << endl;
    return 0;
  }
  std::signal(SIGINT, signal);

  pthread_t msg;
  vector<int> opt = {SO_REUSEADDR, SO_REUSEPORT};
	//cout<<"ARGV:"<<argv[1]<<endl;

  if (tcp.setupSocket(atoi(argv[1]), opt) == 0)
  {
    if(pthread_create(&msg, NULL, receive_client, (void*)0)==0)
    {
      while(1)
      {
        tcp.Accepted();
        cerr << "Accepted" << endl;
      }
    }
  }
  else
  {
    cerr << "socket setup error" << endl;
  }
  return 0;
}
