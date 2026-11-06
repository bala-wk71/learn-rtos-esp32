// Use only one core for demo purpose
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Task Handles
static TaskHandle_t write_output = NULL;
static TaskHandle_t get_input = NULL;

// Added 'volatile' because this variable is shared between two tasks
volatile char *ptr = NULL;

// Task 1
void writeOutput(void *parameter){
  while(1){
    if(ptr!=NULL){
      Serial.println((char*)ptr);
      vPortFree((char*)ptr);
      ptr = NULL;
    }
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

// Task 2
void getInput(void *parameter){
  while(1){
    char text[100];
    if (Serial.available()){
      int index = 0;
      char letter;
      do{
        letter = Serial.read();
        text[index] = letter;
        index++;
      }while(letter!=10);
      index--;
      char *temp = (char*)pvPortMalloc((index + 1)* sizeof(char));
      for(int i=0; i<index; i++){
        temp[i] = text[i];
      }
      temp[index] = '\0';
      ptr = temp;
    }
    // Added a small delay to prevent the watchdog timer from triggering
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup(){
  // FIX 1: Set a standard baud rate. 
  // Make sure your Serial Monitor is also set to 115200!
  Serial.begin(115200);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("--------FreeRTOS LED blink---------");

  Serial.print("Setup and loop tasks on running core ");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority ");
  Serial.println(uxTaskPriorityGet(NULL));

  // FIX 3: Increased stack size to 2048 to prevent crashes
  xTaskCreatePinnedToCore(writeOutput,
                          "task1",
                          1024, 
                          NULL,
                          1,
                          &write_output,
                          app_cpu);
  
  // Increased stack size to 2048 here as well (Serial requires more memory)
  xTaskCreatePinnedToCore(getInput,
                        "task2",
                        1024,
                        NULL,
                        2,
                        &get_input,
                        app_cpu);
}

void loop(){
  // loop() can stay empty when using FreeRTOS tasks
}