ESPr IR 赤外線リモコン データ受信機
====
赤外線リモコンから送信されているデータを解析する

使い方
----

 + スケッチを書き込み、シリアルモニタを開く

 + シリアルモニタに表示されるdumpedData[]をコピーする
   + (例)
      ```c
      unsigned int dumpedData[4] = {
        3400,1600,450,1200
      };
      ```


データ内容
----

 + 配列の要素は単位usの時間
 
 + 信号が検知された時間、されなかった時間、された時間･･･と繰り返し記録されている。

 + `{3400,1600,450,1200}`の場合、3400us間検知され、1600us間検知されず、450us間検知され、1200us間検知されず、といった具合


関連情報
----

1. [ESPr IR 赤外線リモコン](https://www.switch-science.com/catalog/2740/)

2. [スイッチサイエンスマガジン](http://mag.switch-science.com/)

3. [ESPr IR sender](https://github.com/SWITCHSCIENCE/samplecodes/tree/master/ESPr-IR/ESPrIR_sender)