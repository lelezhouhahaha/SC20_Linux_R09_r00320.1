#ifndef TEL_SDK_H
#define TEL_SDK_H
#include <QObject>
#include <QProcess>
#include <pthread.h>


bool start(QObject *parent);

//派生自QObject
class tel_sdk : public QObject
{
   Q_OBJECT
   //注册属性，使之可以在QML中访问--具体语法请参考其他资料
   //Q_PROPERTY(QString name READ getName WRITE setName)
   //Q_PROPERTY(int year READ getYear WRITE setYear NOTIFY yearChanged)

public:
   explicit tel_sdk();
   //通过Q_INVOKABLE宏标记的public函数可以在QML中访问
   //Q_INVOKABLE void sendSignal();//功能为发送信号


    
	//void ql_voice_call_ind_func(unsigned int ind_id,void* ind_data,uint32_t ind_data_len);
   //给类属性添加访问方法--myName
   //void setName(const QString &name);
   //QString getName() const;
   //给类属性添加访问方法--myYear
   //void setYear(int year);
   //int getYear() const;

//signals:
   //信号可以在QML中访问
   //void cppSignalA();//一个无参信号
   //void cppSignalB(const QString &str,int value);//一个带参数信号
   //void yearChanged(int year);

public slots:
   //public槽函数可以在QML中访问
   void cppSlotA(const QString &str);//一个无参槽函数
   void cppSlotB(const QString &str,int value);//一个带参数槽函数
   void dial_stop();
   char display();
   void Call_State();
   void tel_answer();
   //void tel_answer_over();

private:
   //类的属性
   //QString myName;
   //int myYear;
};

#endif // TEL_SDK_H
