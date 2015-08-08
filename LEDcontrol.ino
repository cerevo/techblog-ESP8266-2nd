#include <SoftwareSerial.h>

SoftwareSerial esp8266(7, 8);
#define DEBUG true
#define TIMEOUT 2000

#define ESP_RST 14
#define ESP_EN 2
#define LED1   3 //G
#define LED2   4 //R
#define LED3   9 //B

bool ledstate = true;

void setup() {
  // put your setup code here, to run once:
  pinMode(ESP_RST, OUTPUT);
  pinMode(ESP_EN, OUTPUT);
  pinMode(LED1, OUTPUT);  
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  digitalWrite(LED1,HIGH);
  digitalWrite(LED2,HIGH);
  digitalWrite(LED3,HIGH);

  Serial.begin(9600);
  delay(100);
  Serial.println("ESP_RST");
  digitalWrite(ESP_RST,HIGH);
  delay(100);
  Serial.println("ESP_EN");
  digitalWrite(ESP_EN,HIGH);
  delay(1000);
  
  Serial.begin(9600);

  //your esp's baud rate might be different
  esp8266.begin(115200);                                  

  sendData("AT+RST\r\n",TIMEOUT,DEBUG); // esp8266をソフトウェアリセット

  delay(5000);

  
  sendData("AT+CWMODE=2\r\n",TIMEOUT,DEBUG); // アクセスポイントモードに設定
  sendData("AT+CIFSR\r\n",TIMEOUT,DEBUG); // ローカルIPアドレスの取得
  sendData("AT+CIPMUX=1\r\n",TIMEOUT,DEBUG); // マルチ接続設定ON 1つ以上のアクセスを受け付ける
  sendData("AT+CIPSERVER=1,80\r\n",TIMEOUT,DEBUG); // サーバーモードON. 80番ポートをアクセス可能に

  //LED 緑点灯
  digitalWrite(LED1,LOW);
}

void loop() {
  // put your main code here, to run repeatedly:

  if(esp8266.available()) // check if the esp is sending a message 
  {

    /*
    while(esp8266.available())
    {
      // デバック用
      char c = esp8266.read(); // １文字毎に読んでシリアルに表示
      Serial.write(c);
    } 
  */

    //+IPDを受信するとネットワークデータを受信
    if(esp8266.find("+IPD,"))
    {
     delay(1000);
  
     int connectionId = esp8266.read()-48; // IPDの次の文字がconnection idなので　読み取る
                                           // アスキーで受信されるので、アスキーコードの0が10進で48なので、引いておく
          if(esp8266.find("POST"))         // POSTの文字列を受信するとLEDの色を変える
            {
              Serial.print("POST\r\n");

             
              if(ledstate){

                //LED 赤点灯
                    digitalWrite(LED1,HIGH);
                    digitalWrite(LED2,LOW);
                    digitalWrite(LED3,HIGH);
              }else{

                //LED 青点灯
                  digitalWrite(LED1,HIGH);
                  digitalWrite(LED2,HIGH);
                  digitalWrite(LED3,LOW);
              }

              ledstate = !ledstate;
            }

     //データ本体　ボタンを一つ配置して、ボタンを押すとPOSTを返答する
     String webpage = "HTTP/1.0 200 OK\r\n";
     webpage += "\r\n";
     webpage += "<html><form method=POST><h1>hello</h1><input type=submit value=LED />";
     webpage += "</form></html>";


     //データを送る前に、idとデータのバイト数を<id>,<length>の順に記載する
     String cipSend = "AT+CIPSEND=";
     cipSend += connectionId;
     cipSend += ",";
     cipSend +=webpage.length();
     cipSend +="\r\n";
     
     sendData(cipSend,1000,DEBUG);

      delay(2000);

     //データ本体を送信
     sendData(webpage,1000,DEBUG);

    //現在のconnection idのTCPポートをクローズする
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";
     
     sendData(closeCommand,3000,DEBUG);

     //LED　緑点灯
     digitalWrite(LED1,LOW);
     digitalWrite(LED2,HIGH);
     digitalWrite(LED3,HIGH);   
    }
  }

}

//データ送信関数
//文字列で受けたデータを１文字ずつesp8266に送信
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    char con[256];
    char i;

    command.toCharArray(con,256);
    
    //esp8266.print(command); // send the read character to the esp8266

    for(i=0; i<command.length(); i++){
      esp8266.write(con[i]);
    }
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}
