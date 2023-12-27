var aio_n = "z6256875";
var aio_k = "aio_DOrZ27vARlCBJKqGhqefo4a6DluP";
var line_t = "x1ubYW7hDdOv8umaNbnA/V6I1WfAL9+ZU/1kS1m96r+g2DfsuIoswv7VSyWRUWuTWagnifVspSq/wdgutc68efNNI2lIUw1KOsTUWBObF+BtdwG+ZudTA/uNYnPST4/SclOSRtACblQocvc2Vx9e2AdB04t89/1O/w1cDnyilFU=";
var aio_d = 'voice';
  
var keyWords = {
  '打開': 100,
  '前燈開':100,
  '開燈': 100,
  '關燈': 0,
  '前燈關':0,
  '關閉': 0,
};

function postToAIO(v) {
  var url ='https://io.adafruit.com/api/v2/z6256875/feeds/voice/data';
  response = UrlFetchApp.fetch(url, {
    'headers': {
      'Content-Type': 'application/json; charset=UTF-8',
      'X-AIO-Key': aio_k
    },
    'method': 'post',
    'payload': JSON.stringify({
      'value': v,
    }),
    'muteHttpExceptions': true
  });
  json_response = JSON.parse(response);
  return json_response;
}

function doGet(e) {
  var returnText;
  var val = e.parameter.val;
  if (val) {
    var r = postToAIO(val);
    if (r.value) {
      returnText = r.value + " OK\n";
    }
    else {
      returnText = "Error: post value\n";
    }
  }
  var textOutput = ContentService.createTextOutput(returnText)
  return textOutput;
}

function doPost(e) {
  var msg = JSON.parse(e.postData.contents);

  // 取出 replayToken 和使用者送出的訊息文字
  var replyToken = msg.events[0].replyToken;
  var userMessage = msg.events[0].message.text;

  if (typeof replyToken === 'undefined') {
    return;
  }

  var returnText;
  var hasKeyword = false;
  var val;

  if (userMessage) {
    for (var k in keyWords) {
      if (userMessage.indexOf(k) !== -1) {
        hasKeyword = true;
        val = keyWords[k];
        break;
      }
    }
  }

  if (hasKeyword) {
    var r = postToAIO(val);
    if (r.value) {
      returnText = "已傳送指令";
    }
    else {
      returnText = "傳送指令出錯";
    }
  }
  else {
    returnText = getMisunderstandWords();
  }

  // returnText = userMessage;
  var url = 'https://api.line.me/v2/bot/message/reply';
  UrlFetchApp.fetch(url, {
    'headers': {
      'Content-Type': 'application/json; charset=UTF-8',
      'Authorization': 'Bearer ' + line_t.trim(),
    },
    'method': 'post',
    'payload': JSON.stringify({
      'replyToken': replyToken,
      'messages': [{
        'type': 'text',
        'text': returnText,
      }],
    }),
  });
}


function getMisunderstandWords() {
  var _misunderstandWords = [
    "不好意思，我無法理解您的需求",
    "我不懂您的意思，抱歉我會加強訓練的"
  ];

  if (typeof misunderstandWords === 'undefined') {
    var misunderstandWords = _misunderstandWords;
  }
  else {
    misunderstandWords = misunderstandWords.concat(_misunderstandWords);
  }

  return misunderstandWords[Math.floor(Math.random() * misunderstandWords.length)];
}