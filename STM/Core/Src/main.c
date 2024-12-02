/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#define device_id 0x27<<1
int DHT11_check = 1; // Kiem tra, khoi tao qua trinh doc, setup = 1
uint8_t do_am_byte1, do_am_byte2, temperature_byte1, temperature_byte2; // Luu tru du lieu doc tu DHT11
uint16_t sum; //, temperature, humidity, temperature_sau_dau_phay, humidity_sau_dau_phay; // temperature, humidity co 16 byte theo datasheet
uint8_t test=0; // Bien luu trang thai kiem tra
int temperature, humidity, temperature_sau_dau_phay, humidity_sau_dau_phay;
char lcd_buffer [16];
//int dem=0;
uint8_t check;
uint8_t cnt=0;
// Khai bao cho LCD
uint16_t ADC_val=0;
uint8_t dem=0, check_esp=0;
uint8_t dem1=0;
uint8_t dung=0;
uint8_t check=0;
uint8_t check_new=0;
char mang[10];
char matkhau[10] = {1,2,5,4};
char matkhau_moi[10];
volatile uint8_t kiemtra=0;
char hi[30];
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_UART4_Init(void);
static void MX_I2C3_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
uint8_t send_servo(uint8_t goc) {
    if (goc > 180) goc = 180;  // Gi?i h?n góc t?i da
    float xung = 0.5 + (2.5 - 0.5) * ((float)goc / 180.0);  // Tính th?i gian xung (ms)
    float duty = (xung / 20.0) * 200.0;  // Chuy?n d?i sang duty cycle, 200 là giá tr? max c?a Timer
    return (uint8_t)(duty + 0.5);  // Làm tròn tru?c khi ép ki?u
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void delay_us(uint32_t n){
	volatile uint32_t delay = 24*n;
	volatile uint32_t delayy;
	for(delayy=0; delayy<delay; delayy++);
}

void delay_ms(uint32_t n){
	volatile uint32_t delay;
	volatile uint32_t delayy;
//	for(delayy=0; delayy<delay; delay++);
	
	for(delayy = 0;delayy<n;delayy++){for(volatile uint32_t dl = 0; dl < 24000; dl++);}
}
GPIO_InitTypeDef GPIO_InitStruct;
void Set_pin_out(){
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
void Set_pin_in(){
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;	
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
void DHT11_start(void){
	Set_pin_out();
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,0); //Dua tin hieu xg muc thap
	delay_ms(18);
	Set_pin_in();
}
void check_reponse(void){
	test=0;
	delay_us(40);
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == 0){
		delay_us(80);
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == 1){
			test=1;
		}
	}
	while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == 1); // Cho tin hieu ve muc thap
}
uint8_t read_data ()
{
	uint8_t i=0,j;
	for (j=0;j<8;j++)
	{
		while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_1)) == 0);   // Cho muc cao
		delay_us (40);
//		if ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_1)) == 0)
//		{
//			i&= ~(1<<(7-j));   	
//		}
//		else i|= (1<<(7-j));  // Neu pin cao thi set 1,  
		if ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_1)) == 1){
			i|= (1<<(7-j));
		}
		while ((HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_1)));  //
	}
	return i;
}

// Code cho LCD i2c

void lcd_i2c_send_cmd(uint8_t data){
	uint8_t buf[4] = {(data&0xf0)|0x0c , (data&0xf0)|0x08, (data<<4)|0x0c, (data<<4)|0x08};
	HAL_I2C_Master_Transmit(&hi2c1,device_id,buf,sizeof(buf), 10);
	HAL_I2C_Master_Transmit(&hi2c3,device_id,buf,sizeof(buf), 10);
}
void lcd_i2c_send_cmd1(uint8_t data){
	uint8_t buf[4] = {(data&0xf0)|0x0c , (data&0xf0)|0x08, (data<<4)|0x0c, (data<<4)|0x08};
	HAL_I2C_Master_Transmit(&hi2c3,device_id,buf,sizeof(buf), 10);
}

void lcd_init(){
	lcd_i2c_send_cmd(0x33);
	lcd_i2c_send_cmd(0x32);
	lcd_i2c_send_cmd(0x28);
	lcd_i2c_send_cmd(0x0c);
	lcd_i2c_send_cmd(0x06);
	lcd_i2c_send_cmd(0x01); // Day la lenh xoa man hinh
	//lcd_i2c_send_cmd(0x08); // Bat sang LCD
	lcd_i2c_send_cmd1(0x33);
	lcd_i2c_send_cmd1(0x32);
	lcd_i2c_send_cmd1(0x28);
	lcd_i2c_send_cmd1(0x0c);
	lcd_i2c_send_cmd1(0x06);
	lcd_i2c_send_cmd1(0x01);
	delay_ms(15);
}

void lcd_i2c_send_data(uint8_t data){
	uint8_t buf[4] = {(data & 0xF0) | 0x0D, (data & 0xF0) | 0x09, (data << 4) | 0x0D, (data << 4) | 0x09};
	HAL_I2C_Master_Transmit(&hi2c1, device_id, buf, sizeof(buf), 10);
	delay_ms(2);
}

void lcd_i2c_send_data1(uint8_t data){
	uint8_t buf[4] = {(data & 0xF0) | 0x0D, (data & 0xF0) | 0x09, (data << 4) | 0x0D, (data << 4) | 0x09};
	HAL_I2C_Master_Transmit(&hi2c3,device_id,buf,sizeof(buf), 10);
	delay_ms(2);
}

void lcd_i2c_clear(){
	lcd_i2c_send_cmd(0x01);
	delay_ms(5);
}

void lcd_i2c_clear1(){
	lcd_i2c_send_cmd1(0x01);
	delay_ms(5);
}

void lcd_i2c_set_cs(int x, int y){
	if(x==0){
		lcd_i2c_send_cmd(0x80 + y);
	}else
	if(x==1){
		lcd_i2c_send_cmd(0xC0 + y);
	}
}

void lcd_i2c_set_cs1(int x, int y){
	if(x==0){
		lcd_i2c_send_cmd1(0x80 + y);
	}else
	if(x==1){
		lcd_i2c_send_cmd1(0xC0 + y);
	}
}

void lcd_i2c_send_string(char *string){
	for(int i=0; i<strlen(string); i++){
		lcd_i2c_send_data(string[i]);
	}
}

void lcd_i2c_send_string1(char *string){
	for(int i=0; i<strlen(string); i++){
		lcd_i2c_send_data1(string[i]);
	}
}

void lcd_i2c_send_num(int num){
	char buff[8];
	sprintf(buff, "%d", num);
	lcd_i2c_send_string(buff);
}

void lcd_i2c_send_num1(int num){
	char buff[8];
	sprintf(buff, "%d", num);
	lcd_i2c_send_string1(buff);
}

void lcd_i2c_backlight_on() {
    uint8_t data = 0x08; // B?t backlight
    HAL_I2C_Master_Transmit(&hi2c1, device_id, &data, 1, 10);
}

void lcd_i2c_backlight_off() {
    uint8_t data = 0x00; // B?t backlight
    HAL_I2C_Master_Transmit(&hi2c1, device_id, &data, 1, 10);
}


// KEYPAD

void kiemtra_matkhau(void);
void doi_matkhau(void);
void test_keypad(){
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_RESET);
	// Kiem tra cot 1
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0);
		mang[dem]=1;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == 0);
		mang[dem]=4;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) == 0);
		mang[dem]=7;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 0);
		if (dem > 0) {
			dem--;
      mang[dem] = '\0';  // Xóa ký t? cu?i trong m?ng
            
//            HD44780_SetCursor(dem, 1);  // Di chuy?n con tr?
//            HD44780_PrintStr(" ");  // Xóa ký t? trên màn hình
//            HD44780_SetCursor(dem, 1);  // Ð?t con tr? l?i
			lcd_i2c_set_cs1(1,dem);
			lcd_i2c_send_string1(" ");
			lcd_i2c_set_cs1(1,dem);
     }
	}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, 1);
	
	// Kiem tra cot 2
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, 0);
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0);
		mang[dem]=2;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == 0);
		mang[dem]=5;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) == 0);
		mang[dem]=8;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) == 0);
		mang[dem]=0;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, 1);
	
	// Kiem tra cot 3
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0);
		mang[dem]=3;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == 0);
		mang[dem]=6;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) == 0);
		mang[dem]=9;
		sprintf(hi, "%d", mang[dem]);
		lcd_i2c_set_cs(1,dem);
		lcd_i2c_send_string1(hi);
		delay_ms(200);
		lcd_i2c_set_cs1(1,dem);
		lcd_i2c_send_string1("*");
		dem++;
	}
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) == 0);
		kiemtra_matkhau();
	}
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
	
	// Kiem tra cot 4
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, send_servo(180));
		delay_ms(600);
	}
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0){
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == 0);
		doi_matkhau();
	}
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
	
}

void kiemtra_matkhau(){
    check = 0;
    char vc[20];
    lcd_i2c_clear1();
		lcd_i2c_send_string1("HAY NHAP PASS:");
	
    if (kiemtra <= 3) {
        for (int i = 0; i < 10; i++) {
            if (mang[i] != matkhau[i]) {
                check++;
                break;
            }
        }
        if (check != 0) {
            kiemtra++;
						lcd_i2c_set_cs1(0,0);
						lcd_i2c_send_string1("Sai mat khau");
            sprintf(vc, "Ban con %d co hoi", 4 - kiemtra);
						lcd_i2c_set_cs1(0,1);
						lcd_i2c_send_string1(vc);
            HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
            dem = 0;
            memset(mang, 0, sizeof(mang));
            HAL_Delay(100);
						lcd_i2c_clear1();
						lcd_i2c_set_cs1(0,0);
						lcd_i2c_send_string1("Nhap lai mat khau");
						delay_ms(500);
						lcd_i2c_clear1();
						lcd_i2c_send_string1("HAY NHAP PASS:");
        } else {
            kiemtra = 0;
						lcd_i2c_clear1();
						lcd_i2c_set_cs1(0,0);
						lcd_i2c_send_string1("Dung mat khau");
            
            dem = 0;
            memset(mang, 0, sizeof(mang));
            
            delay_ms(200);  // Hi?n th? thông báo m?t lúc tru?c khi xóa
						lcd_i2c_clear1();
						lcd_i2c_set_cs1(0,0);
						lcd_i2c_send_string1("OPEN THE DOOR");
						__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, send_servo(180));
            delay_ms(700);
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
						__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, send_servo(0));
						lcd_i2c_set_cs1(1, 1);
						lcd_i2c_send_string1("     WELCOME     ");
						delay_ms(1200);
						lcd_i2c_clear1();
						lcd_i2c_send_string1("HAY NHAP PASS:");
        }
    }
}

void doi_matkhau() {
    char matkhau_nhap[10] = {0};
    char matkhau_moi[10] = {0};
    int check = 0;

    lcd_i2c_clear1();
    lcd_i2c_send_string1("Let's change pass");
    delay_ms(1000);
    lcd_i2c_clear1();

    // Bu?c 1: Ki?m tra m?t kh?u cu
    for (int lan = 1; lan <= 3; lan++) {
        lcd_i2c_send_string1("Nhap mat khau:");

        memset(mang, 0, sizeof(mang)); // Xóa d? li?u cu
        test_keypad(); // Ch? nh?p m?t kh?u

        if (strlen(mang) == 0) { // N?u chua nh?p
            lcd_i2c_send_string1("Chua nhap mat khau");
            delay_ms(1000);
            continue; // Quay l?i vòng l?p
        }

        if (strcmp(mang, matkhau) == 0) { // So sánh m?t kh?u
            check = 1; // Ðúng m?t kh?u
            break;
        } else {
            lcd_i2c_clear1();
            lcd_i2c_send_string1("Sai mat khau");
            char thong_bao[20];
            sprintf(thong_bao, "Con %d co hoi", 3 - lan);
            lcd_i2c_send_string1(thong_bao);
            delay_ms(1000);
        }
    }

    if (check == 0) {
        lcd_i2c_send_string1("Sai qua nhieu!");
        delay_ms(1000);
        return; // K?t thúc n?u nh?p sai quá 3 l?n
    }

    // Bu?c 2: Nh?p m?t kh?u m?i
    lcd_i2c_clear1();
    lcd_i2c_send_string1("Nhap mat khau moi:");
    memset(mang, 0, sizeof(mang));
    test_keypad();
    strncpy(matkhau_moi, mang, 10);

    // Luu m?t kh?u m?i
    strncpy(matkhau, matkhau_moi, 10);

    // Thông báo d?i m?t kh?u thành công
    lcd_i2c_clear1();
    lcd_i2c_send_string1("Da doi mk!");
    delay_ms(1000);
    lcd_i2c_clear1();
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_UART4_Init();
  MX_I2C3_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  // B?t d?u PWM trên chân 1 c?a TIM1
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);  // B?t d?u PWM trên chân 1 c?a TIM1
	//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, 0);
	char buffer[100],buffer_1[50], buffer_t[17], buffer_h[17];
	lcd_init();
	lcd_i2c_set_cs(0,0);
	lcd_i2c_send_string("HELLO WORLD");
	lcd_i2c_send_string1("HAY NHAP PASS:");
	//lcd_i2c_clear();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == 1){
//			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15));
//			delay_ms(100);
//			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, send_servo(180));
//			delay_ms(600);
//		}
//		else{
//			delay_ms(200);
//			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, send_servo(0));
//			delay_ms(600);
//		}
		
		test_keypad();
		if(DHT11_check==1){
			DHT11_start();
			check_reponse();
			do_am_byte1 = read_data();
			do_am_byte2 = read_data();
			temperature_byte1 = read_data();
			temperature_byte2 = read_data();
			temperature = temperature_byte1;
			temperature_sau_dau_phay = temperature_byte2;
			humidity = do_am_byte1;
			humidity_sau_dau_phay = do_am_byte2;
			sum = read_data();
//			lcd_i2c_clear();
			lcd_i2c_set_cs(0,0);
			sprintf(buffer,"Temp: %d.%d",temperature,temperature_sau_dau_phay);
			lcd_i2c_send_string(buffer);
			sprintf(buffer_t,"%d.%d",temperature,temperature_sau_dau_phay);
			HAL_UART_Transmit(&huart4,(uint8_t *)buffer_t,sizeof(buffer_t),50);
			delay_ms(200);
			lcd_i2c_set_cs(1,0);
			sprintf(buffer_1,"humidity: %d.%d",humidity,humidity_sau_dau_phay);
			lcd_i2c_send_string(buffer_1);
			sprintf(buffer_h,"%d.%d",humidity,humidity_sau_dau_phay);
			HAL_UART_Transmit(&huart4,(uint8_t *)buffer_h,sizeof(buffer_h),50);
			delay_ms(200);
		}
		DHT11_check++;
		if(DHT11_check == 5){
			DHT11_check = 1;
		}
		
		//Code cam bien quang tro
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 30);
		ADC_val = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
		HAL_GPIO_TogglePin(GPIOD, GREEN_pin_Pin); 
		delay_ms(50);
		if(ADC_val <= 2700){
			delay_ms(600);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, send_servo(180));
		}
		else{
			delay_ms(600);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, send_servo(0));
		}
	}
}	
	
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_0){
		delay_ms(200);
		cnt++;
		if(cnt%2==0){
			HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		}
	}
	if(GPIO_Pin == GPIO_PIN_2){
		delay_ms(200);
		check_esp++;
		if(check_esp%2!=0){
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, send_servo(0));
			delay_ms(600);
		}else{
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, send_servo(180));
			delay_ms(600);
		}
	}
	
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 8399;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 199;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12|GPIO_PIN_14, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_12, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GREEN_pin_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PE8 PE10 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PE12 PE14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD13 PD14 GREEN_pin_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GREEN_pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PE0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
