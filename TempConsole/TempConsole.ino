#include <Commands.h>
#include <NovaSDS011.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"


uint8_t DHTPin = D5;      // what digital pin the DHT22 is connected to
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

int timeSinceLastRead = 0;
/*Put your SSID & Password*/
const char* ssid = "4825";  // Enter SSID here
const char* password = "Richard16";  //Enter Password here

//dht section
float TemperatureF = 0;
float Humidity = 0;
float FeelsLike=0;

float tempF =0;
float tempH=0;
float tempFeelsLike=0;

ESP8266WebServer server(80);

DHT dht(DHTPin, DHTTYPE);

//sds section
#define SDS_PIN_RX D7
#define SDS_PIN_TX D6

NovaSDS011 sds011;

float PM25 =0;
float PM10 =0;


void setup() {
  Serial.begin(9600);
  delay(100);
  
  pinMode(DHTPin, INPUT);
  dht.begin();   
  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

 //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");



sds011.begin(SDS_PIN_RX, SDS_PIN_TX);

  //testDataReportingMode();
  //testDataWorkingMode();
  //testDataDutyCycle();
  //testSetDeviceID(0xAAAA);

  if (sds011.setWorkingMode(WorkingMode::work))
  {
    Serial.println("SDS011 working mode \"Work\"");
  }
  else
  {
    Serial.println("FAIL: Unable to set working mode \"Work\"");
  }

  SDS011Version version = sds011.getVersionDate();

  if (version.valid)
  {
    Serial.println("SDS011 Firmware Vesion:\nYear: " + String(version.year) + "\nMonth: " +
                   String(version.month) + "\nDay: " + String(version.day));
  }
  else
  {
    Serial.println("FAIL: Unable to obtain Software Version");
  }

  if (sds011.setDutyCycle(5))
  {
    Serial.println("SDS011 Duty Cycle set to 5min");
  }
  else
  {
    Serial.println("FAIL: Unable to set Duty Cycle");
  }



}

void loop() {
   //handle web connection
    server.handleClient();
    
 float p25, p10;
  if (sds011.queryData(p25, p10) == QuerryError::no_error)
  {
    PM25 = p25;
    PM10 = p10;
    Serial.println(String(millis() / 1000) + "s:PM2.5=" + String(p25) + ", PM10=" + String(p10));
    delay(1000);
  }
}


void handle_OnConnect() {

  tempF = dht.readTemperature(true); // Gets the values of the temperature
  tempH = dht.readHumidity(); // Gets the values of the humidity 
  tempFeelsLike = dht.computeHeatIndex(TemperatureF, Humidity);

  TemperatureF = tempF;
  Humidity = tempH;
  FeelsLike = tempFeelsLike;
 
  
  server.send(200, "text/html", SendHTML(FeelsLike,Humidity,PM10, PM25, TemperatureF)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float TempFstat,float Humiditystat,float PM10, float PM25, float ActualTemp){

String ptr;
ptr +="<!DOCTYPE html>";
ptr +="<html>";
ptr +="<head>";
ptr +="    <link href=\"https://fonts.googleapis.com/css?family=Open+Sans:300,400,600\" rel=\"stylesheet\">";
ptr +="    <meta charset=\"utf-8\" />";
ptr +="    <title>Weather Report</title>";
ptr +="    <style>";
ptr +="        html {";
ptr +="            font-family: 'Open Sans', sans-serif; display: block; color: #333333;";
ptr +="        }";
ptr +="        .wrapper {";
ptr +="            margin-right: auto; margin-left: auto; padding-right: 10px; padding-left: 10px;";
ptr +="        }";
ptr +="        .humidity-icon {";
ptr +="          border-radius: 50%; text-align: center; vertical-align: central;";
ptr +="        }";
ptr +="        .temperature-icon {";
ptr +="          border-radius: 50%; text-align: center; vertical-align: central;";
ptr +="        }";
ptr +="        .particle-icon {";
ptr +="          border-radius: 50%; text-align: center; vertical-align: central;";
ptr +="        }";
ptr +="";
        ptr +="td img {";
ptr +="            display:inline-block;";
ptr +="            margin-left: auto;";
ptr +="            margin-right: auto;";
ptr +="            width: 96px;";
ptr +="            height: 96px;";
ptr +="        }";
ptr +="        .item-text {";
ptr +="            font-weight: 600;";
ptr +="            padding-left: 0px;";
ptr +="            font-size: 42px;";
ptr +="            text-align: left;";
ptr +="            vertical-align:central;";
ptr +="        }";
ptr +="        .item {";
ptr +="            font-weight: 300;";
ptr +="            color: #f39c12;";
ptr +="            font-size: 120px;";
ptr +="            text-align: right;";
ptr +="        }";
ptr +="        .superscript {";
ptr +="            font-size: 32px;";
ptr +="            font-weight: 600;";
ptr +="        }";
ptr +="";
ptr +="    </style>";
ptr +="</head>";
ptr +="<body>";
ptr +="    <div class=\"wrapper\">";
ptr +="        <table align=\"center\">";
ptr +="            <tr>";
ptr +="                 <td colspan=\"3\" class=\"item-text\" style=\"text-align:center;\">Weather Station</td>";
ptr +="            </tr>";
ptr +="";
ptr +="            <tr>";
ptr +="                <td class=\"temperature-icon\"><img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAFAklEQVR4nN2ba4hVVRTHf3fGOzcy0cKCnInpaUUvMQZ6jUmXog8xEBQSRlFRIAR9miiIEKLGHtKg2Ie+9CBMhSL7FmFRQhoVRNFkUVHUOGk5U2o4mnX6sPZu77s558557PPyD5tz7z57rb322muvvfbjQPG4BNgATAAHgSPAF8BzwNIS5CkMfcAm4B8giEjHgCeAnpJkzA19wA6kkUeAcWAImK/S1cCLwHFVZjPQKEXSnPAC0rCfgSu6lBsGplXZRwqQqxBchvTsLHB5jPI3IsPkL+DMHOUqDONIj44noNmqaB7PRaKCMYE0ZigBzc2KZlfE+3OBxxC/sg/4m2jHGgCTwDrEFxWOQ0qIs4BTnRSFxYrmNyd/AHgF4yyTpjEfDUqKbgLtjKBpqPf/qv9N4GGMMo8iihgB+oF5c8hwHcYSOjAXoU/MhOS5PawRqGcDaAMbgYtV3jZEGT8lqFtPqUHXUjlB93ZaOp0mEGVoJPUDAfBUqhZkRFIFaHPXdIfU/6Z6n8YP/II0vhQnqIWI4/zamFkjALYgYxyy+4HSEMf5DWDm/jBzdxWzDRjMVWqP0EJPW+lN9c7tVdfc+xEr0Dy+plMxtUCUD3B7dStiCSAKGCVaMbWCq4Ck5m4rppbQDWkCDyGbIAGy2FkLtFS5JcCrSPBj+4vaw+7puOY+ygmigH66BzPdpr1aKyCpd58AbnB41FYBSb37KOHevXYKGCSZd7fNPQyVV0ATacAI4r2PEh27u4pxzT0MpSmgD3ga2EunA5srHQdeptPcXT8QZe5hKE0B64jX4GPAr8C7wKPA2RaPNhK22uUvJf6OEJSogElV8TUpaAeRhYodu3dTYtSOEBSkgG5LyDinM03gDOBK4DZgFTKEDiMnPM8jlgLJdoRKxRjJxn43PwDpe7JUJziGGQpp/YBGpRVQxPmbbsRpVl7YcIiiq/0ZYVLn59LlirK2xSvp/PJCpX3ACXcRISmKHAJhkV8cZ1h71DYS9I3aRII+sRzpxekUtDOKdrlXiQpEG3PfZ0MK+o2Kdga5NlMrtDEbJFswW99J0MLsG85SIyVcBPyJCL6ebKFsA3PH6CA1uEg5D/gYEfg1kjd+CHgJOfvXaACvK56fUPHY5V5E0B+ABSnon1H0++nckFmA3AgJgPsyypgbeoDvECFXpeQxH9iueHzmvLtD5X9DRVeIw4iA35PNTHuAB4Hrnfxe4EdVx4oM/HPDekS4J3OsQw+RZ30x9OlQ9EXIDzzyXAOstP7vcOqqFKaQ3lniid8AZmfZzdvrqQ6vmEWESxP0hOEUxe+wldfCBEZe4HMI+N7D0ydINr+TPPH+Hz4VoBc8iz3xO1897eut+jA1zeIqFD4VsEc9l3nid6t6fmTl6QjxW091eFXAbvUc9sCrBdyvfm+28m9Rz6hr9KXCvpGddaNlIbIMft/K60UOYQLg2oz8c8OXiIB3euB1MtJojQcU7z1UNBQGuAszTy/0yHcRskAKVB2VRQOJBAPgDfz4mF7gLcVzF51WUUlcABxABN5ENiU0kO8JA+AP4MLM0hWEFZjIcDvphsPpwNuYyO8mb9IVhJXA70gDpoC7iTc7tIB7MHeUDlDDxmucB3yIOeSYQj6SHkEWNvqu0DnA7ciSep9VfifytVmt0QBWA18R/6bJ58iVm9z3/4qeT69Cen8I+Yx2EfKx06RK7wHvAJ9S0PWY/wDAAxCEpHBewAAAAABJRU5ErkJggg==\" /></td>";
ptr +="                <td class=\"item-text\">Temperature</td>";
ptr +="                <td class=\"item\">";
ptr +=String(ActualTemp);
ptr +="<span class=\"superscript\">&deg;F</span></td>";
ptr +="            </tr>";
ptr +="            <tr>";
ptr +="                <td class=\"temperature-icon\"><img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAABmJLR0QA/wD/AP+gvaeTAAAD/klEQVRoge2YTWwbRRTH/7POGrrGsSMHh1C+VJqGqJW4xESiTdMIhFQqpKpIXGgFh9IrlbgXBAdULhxRESoSOZUT4pILNLITDiRSL4hSSMQBRCI3cRtvEtfe2ZnHIVnHRh17Z+2VLeGftNLs7Hsz7+3um/f0gB49/t+wIEqZbPEnAC8/dEGGhcWTicmWrNLACKj3UOMBgAgnAq5Zx9g325dHvy5ebiYX1IFQef6LrbRT5h9xLj9+9tr6cCPZlh1YXSlidaVYNzf81V/zrazpSudT7ogEr4i4KOOTRrLhfAECBVUd+jx/iFfEO7wiwCsCriPeffyzuyMq+VAcWHvvmZNBdaUjP+COiHBHYO+KuJy/r5LvuhhwK+KM9/Z5ZdcJtyLOqORDcaCVGOCOMF0usH9JuFyYKvmuiwHXcWdqDIfrCAhHzqjk+4Ju1IhWYqBsla/02VGQxAUCAKIZOcA/VMkHzcTVN+wdoU8eTlSfL00lAq0bhJZ/IfbjL6Cbd+rmWs0DOnRdDOjSdXlAl67LA7p0PA/4rTpVdDQGdKpOFR2NAZ2qU0XHYkC36lTRsRjQrTpVdCwGdKtOFR2rhbizW2ESEWjPXSJSVp0q9L8AUfM6x4eMbtWpQsuB8QV7NJOzs83kMjk7O5EtNgzIslW+4nJx1XXEqsvlqsvFVSdeVladKnxXjZnc1hRIfg+gv3Z+7doCJDNx8NLEf1WKjNgbi6f6Qy3sfDkwvmCPMkFLAOIAACGx81seD1bWwfM2JDMRTcdw4PAgHhtLA5Hqh7UNYPznqcRyOOb7CWIixnL2dc94sVPB/dnb4IVSnRjPb8PJb6P0+zpSp0cRsaIA0E8M1wGE1qlrGgMvzduvwevECYnN2V/BN3aqzwkGGMnqPS+UUJj9AxC7c0Q4kZnbfLXNdlfxE8Rve4PS7bU641XwQgk7dzZqdmHngxjnh6YOUE0f9MHyXd8Ll5YLtbfKXqqK9vVGCQe9oXu/1EiyDqdOlp7yrYg290YZ4OpsXtVjtQcc4zq6be2NEuEfb9w3YPk2wkwe2F8D+2s0I4TeKFUT0aNHhvzaAWskVbMJy/nVa39vlHDDG8bGnoCZijVVMVMWYi8M1pr1bVOlPdreG12aTv5AYHMAgIiB5OtHGzphpiykTh+pycaUXZxK3vTrQCi9UWbIiwA2ACASewSD515E4vghRNNxMNMAMw1Eh+JIHn8O6bNHvSwMAPeEZBf9Gg+E1Btdmkz+mcluvQnI7wAMIGLAOjYM61jDE+4eI3b21nT/io4DofZGJ7LFEcnYlyA61UiOwOakpEu3phNaxgchWHM3t/kKyHiLQJMG8DQASOBvBjbPIG/o/PM9evRojX8B9qZzgbqUz+MAAAAASUVORK5CYII=\"/></td>";
ptr +="                <td class=\"item-text\">Feels Like</td>";
ptr +="                <td class=\"item\">";
ptr +=String(TempFstat);
ptr +="<span class=\"superscript\">&deg;F</span></td>";
ptr +="            </tr>";
ptr +="            <tr>";
ptr +="                <td class=\"humidity-icon\"><img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAABmJLR0QA/wD/AP+gvaeTAAAE30lEQVRoge1YTWxUVRT+7psZYBCwDfYHBElR/orSqoSYGBOMC010IRDRSGJExCYqRoqJGo2ZSKJSxRqpGtGQIMgCFi5g48IY4sa4UCKBiPxIEEEDbekUbGfePedz8d7MvLYznc68aWUxX/LmvXfn3jPfd879OWeAKqqooorrHkt2J1ct2ZN89P/mkQ+mWIfb9nbPMKnYMaVEJILms+trr0wEsbHCKdZBrjnvWNfOEZez+C+3jheR5j39a5u/6n+s1HGjRmDOpz3LHNVfSDggQFJBvfP8yw2/lk91JBbuS95k0jgOAkyj+fe2GZfHOnbUCDDlbrWuOtZVWFdgrToieDs85WG/M8D3VVgnqnUa1Y5SxhaMQOO2iytozI8EDEiQ/o+RpJh7ut+c9VM42h6avui5z4FzGKQhABA0wAOn22q/H8v4ghGwwhetVSOuwLoKyUTBVSNWXqgE+ZUJRqHoUlEjQqh3GRHtuvtzxsIJcPVBn7B3WYG4CrEKEXmoEgLO1PduUctlmiWvUFWIsPmS290eSoBYiYvNEPbvuWtyWPKNH1+so+jrWfKqyAkhxPKN+o/+aQghgN8NI50VoiLfhhUQ0UhChTd6Hh9K3hOj06MWb5UvQPQ1sdqbR0T3ILglDPnZ27vnqvJZzUM8KMYqN8764MK8sgSkOuafSAtbRPSAWE2K1aSI7LdptqJjwfkwAiztqyqclCPuz/8hYggVjVnXvDKaraKpRKVR0/lHTSw15QKAOAAwuz8DzHTy2/zbgNr07CuJprwpTNFUotKIDk5+QpVxz+M63OPZNsm2MU4Te7yQvQkXYIWr85DMIya3rVJ0TSF70YkkDwC02pKZKvSPXvi33BTyPgLvLYXsTbwA1ThhhrAN6gDyCptSyN6EC1DFaYCtY/J+NlQ4VcheRQQ07+lfKwRPPDX9QLG+qjwIojWv54Hs1hN8NODBQvZCL+LFu5MzrWUXRT9ZvDs5s1h/h+4OVU2qElR/wfoXRXPvkv2+b1Bs17gJsGndpqJ1alnnpvW9Yv2vvrvgkiqfUSUDJP0dZ4QoVfJpbF9UsMAJdZDd+lnPvXTMD5maAQCp5v6zz9ceLjY20n56jQF3AZgBMDhzMmuizxhdL52LvhnNTtkCViYYPdvQ8zOJO7Jr0dNw9NzlmXchYWxRI5tO1jkRbgLxMICFBAjFSePwkEaxYzTPhxYwb0f3BgW/9LzF4IYCKjdc2Fy/q1zbpaAsAUsTnNRXc/k3gk3MbXVBIeduiCUXnnppQapSRAuhrEXcO+1Sm4g2ifWPfL9WUOulB2L1lr6BaRsrTTYfSo7AnA//jKdt7AzAxqzDcwW//wAQ+Ds+Reafb587UDm6I1FyBAZTkdUq2pjxtlovg/QqNfpRUKjVxmtXsWo8SAdRsgAVPBkg6U0ZP3MUyVVUXruuGw/SQZScSojI8mHbpv8ytCDxv1oenuLoKFmACqcCGL7reE0BQf5yKJhFVgrlCDgOcAWAYafn0OTMP5ePh2ZYBKWvAcrevGWgjmwj8fV4kA6iZAED/bJTlUf8/24C5d/QpEwVR1JTB3aOB+kgyjqJ45tP3Gyd6CGCrbmig8HHI2r0EXQu+qsyNAuj/Gw0cWyS0zv5OQNdR+B2ACBwlOA+1NqdSCxNV4xlFVVUUcV1i/8AeEQWLD9A6A4AAAAASUVORK5CYII=\" /></td>";
ptr +="                <td class=\"item-text\">Humidty</td>";
ptr +="                <td class=\"item\">";
ptr +=String(Humiditystat);
ptr +="<span class=\"superscript\">%</span></td>";
ptr +="            </tr>";
ptr +="            <tr>";
ptr +="                <td class=\"particle-icon\">";
ptr +="                    <img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAABmJLR0QA/wD/AP+gvaeTAAACh0lEQVRoge3ZX4hVVRTH8c80f5wxZ2CKmplwJHHQqaT/MCgYBoHVzEOCIagPIoqPieCDWL2UQoQSQ/jm2wg+Cz1l9hAxBEkDKRGRkzDYP0jswcbRHB/WsYbL3D/n3nvuHet8YXMPey3W/u1zzl577XPJycnJ+a/zTPLbi65mCknDcryMVnyBTnyI7WhP7EuaVfi4iO0A3mmgltS8j0dL2FuS9nxj5KTnTawo49OC8xX45WTJKM7UGuSBOgiplifxSK1BmjmBjfi2iePXxNO4iReaLaQa1uBHnGy2kDR0Yj3ew5+YEDtypkyJDekgDiXXU4ntLOZTtr/xJbbWU2RLCVs/fkF3MvgN9OFXDOCxFOPMYgbXq5OZk5NTjFKL+BV8jscTvyt4CZ9hBN8n/WvxlcguNZcGuI3fcAE/l3NuK2HbjW+wQaTBP7BHTGBUZCYYExPYK7JUrbQlcfoxiaP4pA5xG84QPhDp+5QGbHxZ8RSmxe593zIkSpAdzRZSC4dxqbCz2iy0CU8s8P1d1EnTdRK7GH2itBkWGbAsE3gYu7ATD+F0YjuOrxe0n0SmmsTmeqouYAZvZBV8ECcwh7cyGuMqXs8o9j+8JqrPsQxiz2rQKe5t/KD0GqvmO+kWDTrH94oz74tF7B3ifS5VCRRyxCKTzmo213AZ64rY57AaPSovP/7CncLONHcgLV2iMCvGLWwT5+Vx8brNL+L3Li6KBNEwBoX44Qr99+GYmPT+pG8cz2KleFJ14VVRiZbjI7EnVMoy8SR6xB0n/hh5MJW6ChhRfGHeoxvfiTJ8yXJOHGTa/ZsIepKWZTlRNwbEohsXpcZz+DSxtTZLVDV0LLhu5ofinJz/LXcB8s9xSxfTZcIAAAAASUVORK5CYII=\"/>";
ptr +="                </td>";
ptr +="                <td class=\"item-text\">PM 2.5</td>";
ptr +="                <td class=\"item\">";
ptr +=String(PM25);
ptr +="<span class=\"superscript\">&micro;g/m3</span></td>";
ptr +="            </tr>";
ptr +="            <tr>";
ptr +="                <td class=\"particle-icon\">";
ptr +="                    <img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAABmJLR0QA/wD/AP+gvaeTAAAB+0lEQVRoge2ZQU7CQBSGvxiDC9GYCMId4BjoVkw8BtxBPIAhRG6ja1woLtSdd0BAXEl0MW20j9LOlCkDyXzJSyh5tP8/efPaPsDj8ayDMnANDIHPIIZAByg51KXFBTAGfpbEB9B0pi6FC2DOcvFhzNlAE2WSV17GiAzltGNJbBxt4PDf8VfwXTWIdvBdyBHQylGPMc9EVzhOXEvkDNemToMpUXGVmJwTkTMxvcjuCgLTONDIkSX8Y3qRPPeADpfi+N2JigxUUPU/I1pCVzZOrtv2bMcION5WA3Pg3IZ4FwZGNsWvy8AEeETVvJWy8Xg8f7i6D4TxCbwCXaC+jQbk/aEPFLbVQBj3JiZci10Wt7oGXLMPNIABUQPfQM2hLmMKLJq4caooA6dEDby4lWNOkaiBhVfOuDeyLBvMZNI2Eb/dN/OUzqrdIm3S9ibyGwm5ZyJXq4RstLykSVtX5A6I7/F7wIPI1drEtvr2sklbHdUSpYlTVM0XUSsvxefaRk1f0vuYL0gvL/H/0Z20FVCPB7ri7zB8HspKVVw4adJWQD0eyHKSZdPDknjZ/qoaBsYa562hNucLahw5DT7fYLnmswxrn2wKWJUOUXEzlOAKOU/abFFC3aRM2ujGjUya6P9lZHVYZZMmanWTVn5jxYeUUHviCdU1/KTN43HEL4WFMG3rBSWyAAAAAElFTkSuQmCC\"/>";
ptr +="                </td>";
ptr +="                <td class=\"item-text\">PM 10</td>";
ptr +="                <td class=\"item\">";
ptr +=String(PM10);
ptr +="<span class=\"superscript\">&micro;g/m3</span></td>";
ptr +="            </tr>";
ptr +="        </table>";
ptr +="     </div>";
ptr +="</body>";
ptr +="</html>";
   
  return ptr;
}
