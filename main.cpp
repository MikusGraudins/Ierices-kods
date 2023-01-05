#include <Arduino.h>
#include <AccelStepper.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x26,20,4); 
AccelStepper MotorsX(1, 49, 51); // DIR- (51) ; PUL- (49)

int MaxSpeed = 250; 
int Acceleration = 100;
int Homing_MaxSpeed = 75;
int Homing_Acceleration = 50;

int vanna1_attalums = 40;
int vanna2_attalums = 50;       //attalums mm
int vanna3_attalums = 30;
int vanna4_attalums = 20;

int max_galda_garums = 820;      //maksimālais galda garums, kuru NEDRĪKST pārsniegt (mm)

int vanna1_t = 1;
int vanna2_t = 1; 
int vanna3_t = 1;  //laiks sekundēs
int vanna4_t = 1;

int auto_array[10];
int nospiesto_pogu_skaits = 0;
bool vai_gajiens_paveikts = 1;
bool manualais_cikls_darbojas = 0;
bool rezims = 0; // rezims = 0 - manuali vai 1 - auto
long Homing_d = -1;

int poga1 = 22;
int poga2 = 24;
int poga3 = 26;
int poga4 = 28;
int Start_p = 38; // zaļa
int Stop_p = 36;  // sarkana
int Prog_p = 40;  // zila
int Mode_p = 34;  // dzeltena
int Z_IN1 = 45;
int Z_IN2 = 47;
int Start_LED = 3;
int Mode_LED = 5; // Manual OFF / Auto ON
int Prog_LED = 2; 
int Stop_LED = 4;
int Homing_pin = 9; 
int UP_LS = 7;
int DOWN_LS = 8;
int AS_1 = 10;
int AS_2 = 11;

bool avarija = 0; // vai bijusi avārija
bool stop_aktivizets = 0; // vai stop poga (sarkanā) ir nospiesta
bool cikls_pabeigts = 0;
int veikto_kustibu_skaits;

double solis_l = 0.17;
int vanna1_l = vanna1_attalums/solis_l;
int vanna2_l = vanna2_attalums/solis_l;
int vanna3_l = vanna3_attalums/solis_l;
int vanna4_l = vanna4_attalums/solis_l;
long table_max_l = max_galda_garums/solis_l;


void Mode_LED_Blink()
{
   digitalWrite(Mode_LED, LOW);
   delay(200);
   digitalWrite(Mode_LED, HIGH);
   delay(200);
   digitalWrite(Mode_LED, LOW);
   delay(200);
   digitalWrite(Mode_LED, HIGH);
   delay(200);
   digitalWrite(Mode_LED, LOW);
   delay(200);
   digitalWrite(Mode_LED, HIGH);
   delay(200);
   digitalWrite(Mode_LED, LOW);
   delay(200);
   digitalWrite(Mode_LED, HIGH);
   delay(200);
}

void lcd_auto_manual(){
   lcd.setCursor(0,0);
   lcd.print("                    ");
   if(rezims == 1){
      lcd.setCursor(7,0);
      lcd.print(">AUTO<");
      digitalWrite(Mode_LED, HIGH);
      delay(50);
   }
   if(rezims == 0){
      lcd.setCursor(6,0);
      lcd.print(">MANUAL<");
      digitalWrite(Mode_LED, LOW);
      delay(50);
   }
}

void lcd_sakums(){
   lcd.setCursor(0,0);
   lcd.print("                    ");
   if(rezims == 1){
      lcd.setCursor(7,0);
      lcd.print(">AUTO<");
      digitalWrite(Mode_LED, HIGH);
      delay(50);
   }
   if(rezims == 0){
      lcd.setCursor(6,0);
      lcd.print(">MANUAL<");
      digitalWrite(Mode_LED, LOW);
      delay(50);
   }
   lcd.clear();
   lcd.setCursor(2,2);
   lcd.print("> ATGRIEZAS UZ <");
   lcd.setCursor(2,3);
   lcd.print(">    SAKUMU    <");
}

void Z_augsa()
{
   while (digitalRead(UP_LS) == HIGH)
   {
      delay(50);
      digitalWrite(Z_IN1, LOW);
      digitalWrite(Z_IN2, HIGH);
      if (digitalRead(UP_LS) == LOW)
      {
         digitalWrite(Z_IN1, LOW);
         digitalWrite(Z_IN2, LOW);
         break;
      }
      if(digitalRead(Stop_p) == LOW){
         digitalWrite(Stop_LED, HIGH);
         cikls_pabeigts = 1;
         stop_aktivizets = 1;
         Z_augsa();
         MotorsX.setMaxSpeed(MaxSpeed);
         MotorsX.setAcceleration(Acceleration);
         MotorsX.moveTo(0);
         while (MotorsX.currentPosition() < table_max_l && MotorsX.currentPosition() >= 0)
         {
            MotorsX.run();
            if(MotorsX.currentPosition() == 0){
               digitalWrite(Stop_LED, LOW);
               break;
            }
         }
         break;
      }
      
   }
}

void Z_leja()
{
   while (digitalRead(DOWN_LS) == HIGH)
   {
      delay(50);
      digitalWrite(Z_IN1, HIGH);
      digitalWrite(Z_IN2, LOW);
      if (digitalRead(DOWN_LS) == LOW)
      {
         digitalWrite(Z_IN1, LOW);
         digitalWrite(Z_IN2, LOW);
         break;
      }
      if(digitalRead(Stop_p) == LOW){
         digitalWrite(Stop_LED, HIGH);
         cikls_pabeigts = 1;
         stop_aktivizets = 1;
         digitalWrite(Z_IN1, LOW);
         digitalWrite(Z_IN2, LOW);
         Z_augsa();
         MotorsX.setMaxSpeed(MaxSpeed);
         MotorsX.setAcceleration(Acceleration);
         MotorsX.moveTo(0);
         while (MotorsX.currentPosition() < table_max_l && MotorsX.currentPosition() >= 0)
         {
            MotorsX.run();
            if(MotorsX.currentPosition() == 0){
               digitalWrite(Stop_LED, LOW);
               break;
            }
         }
         break;
      }
   }
}
void doties_uz_sakumu()
{
   lcd_sakums();
   Z_augsa();
   MotorsX.setMaxSpeed(MaxSpeed);
   MotorsX.setAcceleration(Acceleration);
   MotorsX.moveTo(0);
   while (MotorsX.currentPosition() < table_max_l && MotorsX.currentPosition() >= 0 && avarija !=1)
   {
      MotorsX.run();
      if(MotorsX.currentPosition() == 0){
         lcd.clear();
         lcd_auto_manual();
         break;
      }
   }
   vai_gajiens_paveikts = 1;
   stop_aktivizets = 0;
   cikls_pabeigts = 1;

   digitalWrite(Stop_LED, LOW);
   lcd_auto_manual();
   
}

void if_avarija(){
   MotorsX.stop();
   digitalWrite(Start_LED,LOW);
   digitalWrite(Stop_LED,HIGH);
   avarija = 1;
   digitalWrite(Z_IN1, LOW);
   digitalWrite(Z_IN2, LOW);
   lcd.clear();
   delay(50);
   lcd.setCursor(6,1);      
   lcd.print("AVARIJA");
}

void if_stop_p()
{
   MotorsX.stop();
   lcd_auto_manual();
   lcd.setCursor(5,1);
   lcd.print(">STOP<");

   digitalWrite(Stop_LED, HIGH);
   doties_uz_sakumu();
   digitalWrite(Stop_LED, LOW);
   stop_aktivizets = 1;
}

void X_move_vanna1()
{
   vai_gajiens_paveikts = 0;
   MotorsX.moveTo(vanna1_l);
   MotorsX.setMaxSpeed(MaxSpeed);
   MotorsX.setAcceleration(Acceleration);
   while (MotorsX.currentPosition() != vanna1_l && MotorsX.currentPosition() < table_max_l && MotorsX.currentPosition() >= 0 && avarija == 0 && stop_aktivizets == 0){
      Z_augsa();
      MotorsX.run();
      if (MotorsX.currentPosition() == vanna1_l && avarija == 0)
      {
         lcd_auto_manual();
         lcd.setCursor(0,3);
         lcd.print("                   ");
         lcd.setCursor(1,3);
         lcd.print("Vanna 1 sasniegta");

         vai_gajiens_paveikts = 1;
         delay(500);
         Z_leja();
         delay(vanna1_t*1000);
         Z_augsa();
         break;
      }
      if(digitalRead(AS_1) == LOW || digitalRead(AS_2) == LOW){
         if_avarija();
      }
      if(digitalRead(Stop_p) == LOW){
         digitalWrite(Start_LED, LOW);
         digitalWrite(Stop_LED, HIGH);
         vai_gajiens_paveikts = 1;
         stop_aktivizets = 1;
         veikto_kustibu_skaits = 0;
         if_stop_p();
         cikls_pabeigts = 1;
         break;
      }
   }
}
void X_move_vanna2()
{
   vai_gajiens_paveikts = 0;
   MotorsX.moveTo(vanna2_l);
   MotorsX.setMaxSpeed(MaxSpeed);
   MotorsX.setAcceleration(Acceleration);
   while (MotorsX.currentPosition() != vanna2_l && MotorsX.currentPosition() < table_max_l && MotorsX.currentPosition() >= 0 && avarija == 0 && stop_aktivizets == 0){
      Z_augsa();
      MotorsX.run();
      if (MotorsX.currentPosition() == vanna2_l && avarija == 0)
      {
         lcd_auto_manual();
         lcd.setCursor(0,3);
         lcd.print("                   ");
         lcd.setCursor(1,3);
         lcd.print("Vanna 2 sasniegta");

         vai_gajiens_paveikts = 1;
         delay(500);
         Z_leja();
         delay(vanna2_t*1000);
         Z_augsa();
         break;
      }
      if(digitalRead(AS_1) == LOW || digitalRead(AS_2) == LOW){
         if_avarija();
      }
      if(digitalRead(Stop_p) == LOW){
         digitalWrite(Start_LED, LOW);
         digitalWrite(Stop_LED, HIGH);
         vai_gajiens_paveikts = 1;
         stop_aktivizets = 1;
         veikto_kustibu_skaits = 0;
         if_stop_p();
         cikls_pabeigts = 1;
         break;
      }
   }
}

void X_move_vanna3()
{
   vai_gajiens_paveikts = 0;
   MotorsX.moveTo(vanna3_l);
   MotorsX.setMaxSpeed(MaxSpeed);
   MotorsX.setAcceleration(Acceleration);
   while (MotorsX.currentPosition() != vanna3_l && MotorsX.currentPosition() < table_max_l && MotorsX.currentPosition() >= 0 && avarija == 0 && stop_aktivizets == 0){
      Z_augsa();
      MotorsX.run();
      if (MotorsX.currentPosition() == vanna3_l && avarija == 0)
      {
         lcd_auto_manual();
         lcd.setCursor(0,3);
         lcd.print("                   ");
         lcd.setCursor(1,3);
         lcd.print("Vanna 3 sasniegta");

         vai_gajiens_paveikts = 1;
         delay(500);
         Z_leja();
         delay(vanna3_t*1000);
         Z_augsa();
         break;
      }
      if(digitalRead(AS_1) == LOW || digitalRead(AS_2) == LOW){
         if_avarija();
      }
      if(digitalRead(Stop_p) == LOW){
         digitalWrite(Start_LED, LOW);
         digitalWrite(Stop_LED, HIGH);
         vai_gajiens_paveikts = 1;
         stop_aktivizets = 1;
         veikto_kustibu_skaits = 0;
         if_stop_p();
         cikls_pabeigts = 1;
         break;
      }
   }
}

void X_move_vanna4()
{
   vai_gajiens_paveikts = 0;
   MotorsX.moveTo(vanna4_l);
   MotorsX.setMaxSpeed(MaxSpeed);
   MotorsX.setAcceleration(Acceleration);
   while (MotorsX.currentPosition() != vanna4_l && MotorsX.currentPosition() < table_max_l && MotorsX.currentPosition() >= 0 && avarija == 0 && stop_aktivizets == 0){
      Z_augsa();
      MotorsX.run();
      if (MotorsX.currentPosition() == vanna4_l && avarija == 0)
      {
         lcd_auto_manual();
         lcd.setCursor(0,3);
         lcd.print("                   ");
         lcd.setCursor(1,3);
         lcd.print("Vanna 4 sasniegta");

         vai_gajiens_paveikts = 1;
         delay(500);
         Z_leja();
         delay(vanna4_t*1000);
         Z_augsa();
         break;
      }
      if(digitalRead(AS_1) == LOW || digitalRead(AS_2) == LOW){
         if_avarija();
      }
      if(digitalRead(Stop_p) == LOW){
         digitalWrite(Start_LED, LOW);
         digitalWrite(Stop_LED, HIGH);
         vai_gajiens_paveikts = 1;
         stop_aktivizets = 1;
         veikto_kustibu_skaits = 0;
         if_stop_p();
         cikls_pabeigts = 1;
         break;
      }
   }
}

void auto_1()
{
lcd_auto_manual();
lcd.setCursor(2,3);
lcd.print("Darbojas AUTO 1");

      if(cikls_pabeigts == 0){
      X_move_vanna1();
      }
      if(cikls_pabeigts == 0){
      X_move_vanna2();
      }
      if(cikls_pabeigts == 0){
      doties_uz_sakumu();
      }

lcd_auto_manual();
lcd.setCursor(2,2);
lcd.print("AUTO 1 pabeigts");

      if(digitalRead(Stop_p) == LOW){
         if_stop_p();
         cikls_pabeigts = 1;
         stop_aktivizets = 1;
         digitalWrite(Stop_LED, HIGH);
         digitalWrite(Start_LED, LOW);
      }
}

void Homing_cikls()
{
   if(digitalRead(Start_p) == LOW){
   lcd.clear();
   delay(50);
   Z_augsa();
   digitalWrite(Stop_LED,HIGH);

   MotorsX.setMaxSpeed(Homing_MaxSpeed);
   MotorsX.setAcceleration(Homing_Acceleration);
   lcd.clear();
   lcd.print("Notiek homings...");

   while (digitalRead(Homing_pin))
   {
      MotorsX.moveTo(Homing_d);
      Homing_d++;
      MotorsX.run();
      delay(5);
   }

   MotorsX.setCurrentPosition(0);
   MotorsX.setMaxSpeed(Homing_MaxSpeed);
   MotorsX.setAcceleration(Homing_Acceleration);

   while (!digitalRead(Homing_pin))
   {
      MotorsX.moveTo(Homing_d);
      MotorsX.run();
      Homing_d--;
      delay(5);
   }
   MotorsX.setCurrentPosition(0);
   MotorsX.setMaxSpeed(MaxSpeed);
   MotorsX.setAcceleration(Acceleration);
   lcd.clear();
   lcd.print("Homings pabeigts!");
   delay(2000);
   digitalWrite(Stop_LED,LOW);
   }
   else{
      lcd.setCursor(0,0);
      lcd.print("Nospied START");
      Homing_cikls();
   }
}

void setup()
{
   lcd.init();
   lcd.backlight();
   Serial.begin(9600);
   Serial.println(digitalRead(AS_1));
   Serial.println(digitalRead(AS_2));
   manualais_cikls_darbojas = 0;
   pinMode(poga1, INPUT_PULLUP);
   pinMode(poga2, INPUT_PULLUP);
   pinMode(poga3, INPUT_PULLUP);
   pinMode(poga4, INPUT_PULLUP);
   pinMode(Start_p, INPUT_PULLUP);
   pinMode(Stop_p, INPUT_PULLUP);
   pinMode(Prog_p, INPUT_PULLUP);
   pinMode(Mode_p, INPUT_PULLUP);
   pinMode(Start_LED, OUTPUT);
   pinMode(Mode_LED, OUTPUT);
   pinMode(Prog_LED, OUTPUT);
   pinMode(Stop_LED, OUTPUT);
   pinMode(UP_LS, INPUT_PULLUP);
   pinMode(DOWN_LS, INPUT_PULLUP);
   pinMode(Z_IN1, OUTPUT);
   pinMode(Z_IN2, OUTPUT);
   pinMode(AS_1,INPUT_PULLUP);
   pinMode(AS_2,INPUT_PULLUP);

   Homing_cikls();

   MotorsX.setCurrentPosition(0);
   MotorsX.setMaxSpeed(MaxSpeed);
   MotorsX.setAcceleration(Acceleration);

   lcd_auto_manual();
}

void loop()
{

   while (rezims == 0)
   {
      if (digitalRead(Mode_p) == LOW)
      {
         delay(50);
         rezims = 1;
         digitalWrite(Mode_LED, HIGH);
         lcd.setCursor(0,0);
         lcd.print("                    ");
         lcd.setCursor(7,0);
         lcd.print(">AUTO<");
         delay(500);
      }
      while (digitalRead(Prog_p) == LOW)
      {
         if (auto_array[0] < 1)
         {
            if (digitalRead(poga1) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[0] = 1;
               lcd_auto_manual();
               lcd.setCursor(0,1);
               lcd.print("Vannu seciba:");
               lcd.setCursor(2,2);
               lcd.print("1");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga2) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[0] = 2;
               lcd_auto_manual();
               lcd.setCursor(0,1);
               lcd.print("Vannu seciba:");
               lcd.setCursor(2,2);
               lcd.print("2");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga3) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[0] = 3;
               lcd_auto_manual();
               lcd.setCursor(0,1);
               lcd.print("Vannu seciba:");
               lcd.setCursor(2,2);
               lcd.print("3");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga4) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[0] = 4;
               lcd_auto_manual();
               lcd.setCursor(0,1);
               lcd.print("Vannu seciba:");
               lcd.setCursor(2,2);
               lcd.print("4");
               digitalWrite(Prog_LED, HIGH);
            }
            delay(400);
            digitalWrite(Prog_LED, LOW);
         }
         if (auto_array[0] >= 1 && auto_array[1] < 1)
         {
            if (digitalRead(poga1) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[1] = 1;
               lcd.setCursor(3,2);
               lcd.print("-1");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga2) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[1] = 2;
               lcd.setCursor(3,2);
               lcd.print("-2");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga3) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[1] = 3;
               lcd.setCursor(3,2);
               lcd.print("-3");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga4) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[1] = 4;
               lcd.setCursor(3,2);
               lcd.print("-4");
               digitalWrite(Prog_LED, HIGH);
            }
            delay(400);
            digitalWrite(Prog_LED, LOW);
         }
         if (auto_array[1] >= 1 && auto_array[2] < 1)
         {
            if (digitalRead(poga1) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[2] = 1;
               lcd.setCursor(5,2);
               lcd.print("-1");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga2) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[2] = 2;
               lcd.setCursor(5,2);
               lcd.print("-2");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga3) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[2] = 3;
               lcd.setCursor(5,2);
               lcd.print("-3");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga4) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[2] = 4;
               lcd.setCursor(5,2);
               lcd.print("-4");
               digitalWrite(Prog_LED, HIGH);
            }
            delay(400);
            digitalWrite(Prog_LED, LOW);
         }
         if (auto_array[2] >= 1 && auto_array[3] < 1)
         {
            if (digitalRead(poga1) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[3] = 1;
               lcd.setCursor(7,2);
               lcd.print("-1");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga2) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[3] = 2;
               lcd.setCursor(7,2);
               lcd.print("-2");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga3) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[3] = 3;
               lcd.setCursor(7,2);
               lcd.print("-3");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga4) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[3] = 4;
               lcd.setCursor(7,2);
               lcd.print("-4");
               digitalWrite(Prog_LED, HIGH);
            }
            delay(400);
            digitalWrite(Prog_LED, LOW);
         }
         if (auto_array[3] >= 1 && auto_array[4] < 1)
         {
            if (digitalRead(poga1) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[4] = 1;
               lcd.setCursor(9,2);
               lcd.print("-1");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga2) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[4] = 2;
               lcd.setCursor(9,2);
               lcd.print("-2");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga3) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[4] = 3;
               lcd.setCursor(9,2);
               lcd.print("-3");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga4) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[4] = 4;
               lcd.setCursor(9,2);
               lcd.print("-4");
               digitalWrite(Prog_LED, HIGH);
            }
            delay(400);
            digitalWrite(Prog_LED, LOW);
         }
         if (auto_array[4] >= 1 && auto_array[5] < 1)
         {
            if (digitalRead(poga1) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[5] = 1;
               lcd.setCursor(11,2);
               lcd.print("-1");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga2) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[5] = 2;
               lcd.setCursor(11,2);
               lcd.print("-2");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga3) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[5] = 3;
               lcd.setCursor(11,2);
               lcd.print("-3");
               digitalWrite(Prog_LED, HIGH);
            }
            else if (digitalRead(poga4) == LOW)
            {
               nospiesto_pogu_skaits += 1;
               auto_array[5] = 4;
               lcd.setCursor(11,2);
               lcd.print("-4");
               digitalWrite(Prog_LED, HIGH);
            }
            delay(400);
            digitalWrite(Prog_LED, LOW);
         }
      }
      if (digitalRead(Start_p) == LOW && rezims == 0 && auto_array[0] != 0)
      {
         stop_aktivizets = 0;
         manualais_cikls_darbojas = 1;
         int veikto_kustibu_skaits = 0;
         digitalWrite(Start_LED, HIGH);
         doties_uz_sakumu();
         while (manualais_cikls_darbojas == 1 && veikto_kustibu_skaits < nospiesto_pogu_skaits)
         {
            for (int i = 0; auto_array[i] > 0; i++)
            {
               if (auto_array[i] == 1 && vai_gajiens_paveikts == 1)
               {
                  lcd.setCursor(0,3);
                  lcd.print("                    ");
                  lcd.setCursor(0,3);
                  lcd.print("Apstrades vanna - 1");
                  X_move_vanna1();
                  veikto_kustibu_skaits += 1;
               }
               else if (auto_array[i] == 2 && vai_gajiens_paveikts == 1)
               {
                  lcd.setCursor(0,3);
                  lcd.print("                    ");
                  lcd.setCursor(0,3);
                  lcd.print("Apstrades vanna - 2");
                  X_move_vanna2();
                  veikto_kustibu_skaits += 1;
               }
               else if (auto_array[i] == 3 && vai_gajiens_paveikts == 1)
               {
                  lcd.setCursor(0,3);
                  lcd.print("                    ");
                  lcd.setCursor(0,3);
                  lcd.print("Apstrades vanna - 3");
                  X_move_vanna3();
                  veikto_kustibu_skaits += 1;
               }
               else if (auto_array[i] == 4 && vai_gajiens_paveikts == 1)
               {
                  lcd.setCursor(0,3);
                  lcd.print("                    ");
                  lcd.setCursor(0,3);
                  lcd.print("Apstrades vanna - 4");
                  X_move_vanna4();
                  veikto_kustibu_skaits += 1;
               }
               if (nospiesto_pogu_skaits == veikto_kustibu_skaits)
               {
                  vai_gajiens_paveikts = 1;
                  manualais_cikls_darbojas = 0;
                  doties_uz_sakumu();           
                  digitalWrite(Start_LED, LOW); 
                  lcd.setCursor(0,3);
                  lcd.print("                    ");
                  lcd.setCursor(1,3);
                  lcd.print("Apstrade pabeigta");
                  veikto_kustibu_skaits = 0;
                  break;
               }
         }
      }
   }
   while (rezims == 1)
   {
      if (digitalRead(Mode_p) == LOW)
      {
         delay(50);
         rezims = 0;
         digitalWrite(Mode_LED, LOW);
         lcd.setCursor(0,0);
         lcd.print("                    ");
         lcd.setCursor(6,0);
         lcd.print(">MANUAL<");
         delay(500);
      }

      int izveletais_auto_rezims;

      if (digitalRead(poga1) == LOW)
      {
         izveletais_auto_rezims = 1;
         lcd.setCursor(0,2);
         lcd.print("Darba cikls: AUTO 1");
         Mode_LED_Blink();
         delay(500);
      }
      if (digitalRead(poga2) == LOW)
      {
         izveletais_auto_rezims = 2;
         lcd.setCursor(0,2);
         lcd.print("Darba cikls: AUTO 2");
         Mode_LED_Blink();
         delay(500);
      }
      if (digitalRead(poga3) == LOW)
      {
         izveletais_auto_rezims = 3;
         lcd.setCursor(0,2);
         lcd.print("Darba cikls: AUTO 3");
         Mode_LED_Blink();
         delay(500);
      }
      if (digitalRead(poga4) == LOW)
      {
         izveletais_auto_rezims = 4;
         lcd.setCursor(0,2);
         lcd.print("Darba cikls: AUTO 4");
         Mode_LED_Blink();
         delay(500);
      }
      if (digitalRead(Start_p) == LOW)
      {
         cikls_pabeigts = 0;
         if (izveletais_auto_rezims == 1 && cikls_pabeigts == 0)
         {
            digitalWrite(Start_LED,HIGH);
            auto_1();
            if(digitalRead(Stop_p) == LOW){
               if_stop_p();
               cikls_pabeigts = 1;
               break;
            }
            digitalWrite(Start_LED,LOW);
            cikls_pabeigts = 1;
         }
         if (izveletais_auto_rezims == 2 && cikls_pabeigts == 0)
         {
            digitalWrite(Start_LED,HIGH);
            // auto_2();
            digitalWrite(Start_LED,LOW);
            cikls_pabeigts = 1;
         }
         if (izveletais_auto_rezims == 3 && cikls_pabeigts == 0)
         {
            digitalWrite(Start_LED,HIGH);
            // auto_3();
            digitalWrite(Start_LED,LOW);
            cikls_pabeigts = 1;
         }
         if (izveletais_auto_rezims == 4 && cikls_pabeigts == 0)
         {
            digitalWrite(Start_LED,HIGH);
            // auto_4();
            digitalWrite(Start_LED,LOW);
            cikls_pabeigts = 1;
         }
         delay(200);
      }
   }
}
}