// Use only one core for demo purpose
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static int LED_PIN = 2;
static int QUEUE_LEN = 5;

// Task Handles
static TaskHandle_t blink_led = NULL;
static TaskHandle_t set_delay = NULL;

QueueHandle_t taskA;
QueueHandle_t taskB;

// Task 1
void blinkLED(void *parameter){
  int blink_delay = 1000;
  int counter = 0;
  while(1){
    int tempNum;
    BaseType_t status = xQueueReceive(taskA, &tempNum, 0);
    if (status == pdPASS){
      blink_delay = tempNum;
    }
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(blink_delay / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(blink_delay / portTICK_PERIOD_MS);
    counter++;

    if (counter%100 == 0){
      const char* reply = "BLINKED";
      xQueueSend(taskB, &reply, 0);
    }
  }
}

// Task 2
void getInput(void *parameter){
  while(1){
    if (Serial.available()){
      String input = Serial.readStringUntil('\n');
      input.trim();

      if(input.startsWith("delay ")){
        String number = input.substring(6);
        int new_delay = number.toInt();
        if (new_delay>0){
          xQueueSend(taskA, &new_delay, 0);
        }
        Serial.printf("Sent delay: %d\n", new_delay);
      }
      else{
        Serial.printf("Got Message: %s\n", input.c_str());
      }

    }

    const char* tempStr;
    BaseType_t status = xQueueReceive(taskB, &tempStr, pdMS_TO_TICKS(10));
    if (status == pdPASS){
      Serial.println((char*)tempStr);
    }

    // commented out the delay since the queue wait 10 ms and this way ti feels faster
    // vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup(){
  // FIX 1: Set a standard baud rate. 
  // Make sure your Serial Monitor is also set to 115200!
  Serial.begin(115200);

  // FIX 2: Set the LED pin as an output
  pinMode(LED_PIN, OUTPUT);

  taskA = xQueueCreate(QUEUE_LEN, sizeof(int));
  taskB = xQueueCreate(QUEUE_LEN, sizeof(char*));

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("--------FreeRTOS LED blink QUEUE---------");

  Serial.print("Setup and loop tasks on running core ");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority ");
  Serial.println(uxTaskPriorityGet(NULL));

  xTaskCreatePinnedToCore(blinkLED,
                          "task1",
                          2048, 
                          NULL,
                          1,
                          &blink_led,
                          app_cpu);

  xTaskCreatePinnedToCore(getInput,
                        "task2",
                        2048,
                        NULL,
                        2,
                        &set_delay,
                        app_cpu);
}

void loop(){
  // loop() can stay empty when using FreeRTOS tasks
}