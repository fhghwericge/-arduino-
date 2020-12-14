//红外相关！！！
#include <IRremote.h>
IRrecv irrecv(11);
decode_results results; 
/**************************/

//蓝牙！！！
#include <SoftwareSerial.h>
SoftwareSerial BT(9, 10); //设定神奇的软串口，9RX，10TX
/****************************/

//利用DS3231模块获取时间！！！
#include <Wire.h>
#include "DS3231.h"
/***********************/

//OLED！！！
#include <U8glib.h>
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
/*******************************************/
const unsigned char PROGMEM bt[] = {
  0x80,0x00,0x00,0x80,0x01,0x00,0x80,0x02,0x00,0x80,0x04,0x00,0x84,0x08,0x00,0x88,
  0x10,0x00,0x90,0x08,0x00,0xA0,0x04,0x00,0xC0,0x02,0x00,0x80,0x01,0x00,0x80,0x01,
  0x00,0xC0,0x02,0x00,0xA0,0x04,0x00,0x90,0x08,0x00,0x88,0x10,0x00,0x84,0x08,0x00,
  0x80,0x04,0x00,0x80,0x02,0x00,0x80,0x01,0x00,0x80,0x00,0x00

};
const unsigned char PROGMEM ir[] = {
  0x00,0x40,0x00,0x00,0x80,0x00,0x00,0x86,0x01,0x00,0x0C,0x01,0x00,0x18,0x03,0x80,
  0x10,0x02,0x8C,0x31,0x02,0x3C,0x21,0x06,0x24,0x23,0x04,0x64,0x22,0x04,0x64,0x22,
  0x04,0x24,0x21,0x06,0xB4,0x21,0x02,0xDC,0x30,0x02,0x40,0x18,0x03,0x00,0x0C,0x01,
  0x00,0x86,0x01,0x00,0xC0,0x00,0x00,0x60,0x00,0x00,0x30,0x00  
};
const unsigned char PROGMEM el[]{
  0xC0,0x3F,0x00,0x60,0xE0,0x00,0x38,0x80,0x03,0x0C,0x00,0x06,0x06,0x00,0x04,0x43,
0x20,0x04,0xA1,0x50,0x0C,0x11,0x89,0x08,0x01,0x00,0x08,0x01,0x00,0x08,0x01,0x00,
0x08,0x01,0x00,0x08,0x41,0x20,0x08,0x83,0x10,0x08,0x06,0x0F,0x0C,0x0C,0x00,0x04,
0x18,0x00,0x06,0x70,0x00,0x03,0xC0,0xC1,0x01,0x00,0x77,0x00
};

//追加！DHT11！！！
#include "DHT.h"
DHT dht(2, DHT11);



int Mytime[5]={0};
int Myhr=999;
int Mymn=999;//初始化时间
int curtain=0;//窗帘状态
int i=0;
int flag1=0;//判断红外接收是否有效的旗子1
int dT=10000;//拉窗帘的时间
int flag2=0;//判断接收方式是红外/蓝牙的旗子2

RTClib RTC;

void setup() {
  //红、蓝指示灯输出口
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  //12,13为电机输出口
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
/*********************/
  Serial.begin(9600);
  Wire.begin();
  irrecv.enableIRIn(); // 初始化红外接收器
  BT.begin(38400);//我们的HC-05波特率是38400
  dht.begin();//DHT11
}

void loop() {
  draw();
  delay(950);
  //如果接收到蓝牙信号，则切换为蓝牙模式，蓝灯亮
  //如果接收到遥控器“CH+”信号，则切换为红外模式，红灯亮
  //注意：每次切换模式都将重制红外输入的数组
  if(flag2==0){
    digitalWrite(7,LOW);
    digitalWrite(8,LOW);      
  }
  if(flag2 != 1){
    if(BT.available()){
      BT.println("Welcome! Input '?' for more help!");
      flag2=1;
      for(int j=0;j<5;j++) Mytime[j]=0;
      i=0;
    }
  }
  if(flag2 != 2){
    if(irrecv.decode(&results)){
      if(results.value==16769565){
        flag2=2;
        for(int j=0;j<5;j++) Mytime[j]=0;
        i=0;
      }
      irrecv.resume();
    }
  }
  if(flag2==1){  
    int m=0;
    if(m=BT.read()){
      digitalWrite(8,HIGH);
      digitalWrite(7,LOW);
      if(m<58){
        Mytime[i]=m-48;
        i++;
      }else if(m==71||m==103){
        Go();
        for(int j=0;j<5;j++) Mytime[j]=0;
        i=0;
      }else if(m==70||m==102){
        Fango();
        for(int j=0;j<5;j++) Mytime[j]=0;
        i=0;
      }else if(m==83||m==115){
        Stop();
        for(int j=0;j<5;j++) Mytime[j]=0;
        i=0;
      }else if(m==63){
         BT.println("Welcome!!!");
         BT.println("Input 4 numbers to set an alarm!");
         BT.println("U can also give a direct command!");
         BT.println("'G'=>Go 'F'=>Fango 'S'=>Stop (Case insensitive)");
        }
      if(i==4){
        Myhr=Mytime[0]*10+Mytime[1];
        Mymn=Mytime[2]*10+Mytime[3];
        for(int j=1;j<5;j++) Mytime[j]=0;
        i=0;
        BT.print("Your clock is successfully set at");
        BT.print(Myhr);
        BT.print(":");
        BT.println(Mymn);
        //指示灯闪烁！！！
        digitalWrite(8,LOW);
        delay(100);
        digitalWrite(8,HIGH);
        delay(100);
        digitalWrite(8,LOW);
        delay(100);
        digitalWrite(8,HIGH);
        delay(100);
        flag2=0;
        /**********************/
      }
    }
  }
  
  if(flag2==2){
    digitalWrite(7,HIGH);
    digitalWrite(8,LOW);//七上八下    
    Mytime[i]=getCode();
    if(flag1==1){
      /*Serial.print(i);
      Serial.print("  ");
      Serial.println(Mytime[i]);*/
      digitalWrite(7,LOW);
      delay(100);
      digitalWrite(7,HIGH);
      i++;
    }
    //重置
    if(i==5){
      i=0;
      Serial.println("Oh!!!"); 
    }
  }
  
  checka(Myhr,Mymn,curtain);  
}




void checka(int a, int b, int curtain){
  //利用神奇的DS3231获取时间！！！
  DateTime now = RTC.now();
  Serial.print(now.hour(),DEC);
  Serial.print(":");  
  Serial.print(now.minute(),DEC);
  Serial.print(":");
  Serial.println(now.second(),DEC);   
  //判断是否到达预定时间！！！
  if(now.hour()==a && now.minute()==b&&now.second()==0){
    if(curtain==0){
      Go();
      delay(dT);
      Stop();
    }else{
      Fango();      
      delay(dT);
      Stop();
    }     
  }
}

//红外解码！！！
int getCode(){
  int result=-1;
  if(irrecv.decode(&results)){
    switch(results.value){
      
    //以下为0~9键表示数字！！！      
      case 16738455: 
        result=0;
        break;
      case 16724175: 
        result=1;
        break;
      case 16718055: 
        result=2;
        break; 
      case 16743045: 
        result=3;
        break;
      case 16716015: 
        result=4;
        break;
      case 16726215: 
        result=5;
        break;
      case 16734885: 
        result=6;
        break;
      case 16728765: 
        result=7;
        break;
      case 16730805: 
        result=8;
        break;
      case 16732845: 
        result=9;
        break;

//"CH"键提交数据！！！
      case 16736925: 
        Serial.println("OKK");
        Myhr=Mytime[0]*10+Mytime[1];
        Mymn=Mytime[2]*10+Mytime[3];
        for(int j=0;j<5;j++) Mytime[j]=0;
        i=0;
        digitalWrite(7,LOW);
        delay(100);
        digitalWrite(7,HIGH);
        delay(100);
        digitalWrite(7,LOW);
        delay(100);
        digitalWrite(7,HIGH);
        delay(100);
        flag2=0;
        break;
        
//"CH-"键清空数据！！！
      case 16753245:
        for(int j=0;j<5;j++) Mytime[j]=0;
        Serial.println("Try again!");
        i=0;
        break;

//直控键：第二行的三个按钮分别控制：正转，反转，刹车。
      case 16720605:
        Go();
        for(int j=0;j<5;j++) Mytime[j]=0;
        i=0;
        break;
      case 16712445:
        Fango();
        for(int j=0;j<5;j++) Mytime[j]=0;
        i=0;
        break;
      case 16761405:
        Stop();
        for(int j=0;j<5;j++) Mytime[j]=0;
        i=0;
        break;
    }
    if(result>=0) flag1=1;
  }else flag1=0;
  irrecv.resume();
  return result;
}

//以下是操控函数！
void Go(){
  Serial.println("I`m Spinning");
  digitalWrite(12,HIGH);
  digitalWrite(13,LOW);
  curtain=1;
}
void Stop(){
  Serial.println("Stop!");
  digitalWrite(12,LOW);
  digitalWrite(13,LOW);
  curtain=0;
}
void Fango(){
  Serial.println("gninnipS m`I");
  digitalWrite(12,LOW);
  digitalWrite(13,HIGH);
}
/**************************************/


//U8glib
void draw(){
   u8g.firstPage();
   do {  
        u8g.setFont(u8g_font_6x13);
        
        for(int j=0;j<i;j++){ 
          u8g.setPrintPos(4+10*j, 26);//选择位置
          u8g.print(Mytime[j]);
        }
        for(int j=i;j<4;j++){
          u8g.drawStr(4+10*j, 26, "_");
        }      
        DateTime now = RTC.now();
        u8g.setPrintPos(5, 45);
        u8g.print(now.year()-2000,DEC);
        u8g.print("\\"); 
        u8g.print(now.month(),DEC);
        u8g.print("\\");
        u8g.print(now.day(),DEC);

        u8g.setPrintPos(5, 60);
        if(now.hour()<10) u8g.print("0");
        u8g.print(now.hour(),DEC);
        u8g.print(":"); 
        if(now.minute()<10) u8g.print("0"); 
        u8g.print(now.minute(),DEC);
        u8g.print(":");
        if(now.second()<10) u8g.print("0");
        u8g.print(now.second(),DEC);

        u8g.drawRFrame(0 ,0 ,128 ,64 ,3);
        u8g.drawVLine(55,0, 64); 
        u8g.setPrintPos(60, 45);
        u8g.print("Tem:");
        u8g.print(dht.readTemperature());
        u8g.print(char(134));
        u8g.print("C");
        u8g.setPrintPos(60, 60);
        u8g.print("Hum:");
        u8g.print(dht.readHumidity());
        u8g.print("%");

        
        if(flag2==1){
          u8g.drawXBMP( 96, 10, 20, 20, bt);
        } else if(flag2==2){
          u8g.drawXBMP( 96, 10, 20, 20, ir);
        } else{
          u8g.drawXBMP( 96, 10, 20, 20, el);
        }
        
           
    }while( u8g.nextPage() );//u8glib规定写法
  
  
} 
