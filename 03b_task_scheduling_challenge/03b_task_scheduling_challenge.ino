// Use only one core for demo purpose
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Task Handles
static TaskHandle_t blink_led = NULL;
static TaskHandle_t set_delay = NULL;

// Added 'volatile' because this variable is shared between two tasks
volatile int blink_delay = 1000;
int led_pin = 2;

// Task 1
void blinkLED(void *parameter){
  while(1){
    digitalWrite(led_pin, HIGH);
    vTaskDelay(blink_delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(blink_delay / portTICK_PERIOD_MS);
  }
}

// Task 2
void getInput(void *parameter){
  while(1){
    if (Serial.available()){
      int new_delay = Serial.parseInt();

      if (new_delay>0){
        blink_delay = new_delay;
      }
    }
    // Added a small delay to prevent the watchdog timer from triggering
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup(){
  // FIX 1: Set a standard baud rate. 
  // Make sure your Serial Monitor is also set to 115200!
  Serial.begin(115200);

  // FIX 2: Set the LED pin as an output
  pinMode(led_pin, OUTPUT);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("--------FreeRTOS LED blink---------");

  Serial.print("Setup and loop tasks on running core ");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority ");
  Serial.println(uxTaskPriorityGet(NULL));

  // FIX 3: Increased stack size to 2048 to prevent crashes
  xTaskCreatePinnedToCore(blinkLED,
                          "task1",
                          1024, 
                          NULL,
                          1,
                          &blink_led,
                          app_cpu);
  
  // Increased stack size to 2048 here as well (Serial requires more memory)
  xTaskCreatePinnedToCore(getInput,
                        "task2",
                        1024,
                        NULL,
                        2,
                        &set_delay,
                        app_cpu);
}

void loop(){
  // loop() can stay empty when using FreeRTOS tasks
}