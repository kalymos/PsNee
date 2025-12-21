
#pragma once

#ifdef BIOS_PATCH

  volatile uint8_t impulse = 0;
  volatile uint8_t patch = 0;

  #ifdef INTERRUPT_RISING

    ISR(PIN_AX_INTERRUPT_VECTOR) {
      impulse++;                         
      if (impulse == TRIGGER){           // If impulse reaches the value defined by TRIGGER, the following actions are performed:
        _delay_us(HOLD);                            
        PIN_DX_OUTPUT;                   
        _delay_us(PATCHING);                       
        PIN_DX_INPUT;                      
        PIN_AX_INTERRUPT_DISABLE;          

        impulse = 0;                    
        patch = 1;                       // patch is set to 1, indicating that the first patch is completed.
      }
    }

    void Bios_Patching(){

        PIN_AX_INTERRUPT_RISING;             

      if (PIN_AX_READ != 0)                 // If the AX pin is high
      {
        while (PIN_AX_READ != 0);           // Wait for it to go low
        while (PIN_AX_READ == 0);           // Then wait for it to go high again.
      }
      else                                  // If the AX pin is low
      {
        while (PIN_AX_READ == 0);           // Wait for it to go high.
      }
      
     // Wait until the number of microseconds elapsed reaches a value defined by CHECKPOINT.
      _delay_ms(CHECKPOINT);                   
      PIN_AX_INTERRUPT_ENABLE;              
      
      while (patch != 1);                   // Wait for the first stage of the patch to complete:

    }

  #endif

  #ifdef INTERRUPT_FALLING

    ISR(PIN_AX_INTERRUPT_VECTOR) {
      impulse++;                         
      if (impulse == TRIGGER){           // If impulse reaches the value defined by TRIGGER, the following actions are performed:
        _delay_us (HOLD);                          
        PIN_DX_OUTPUT;                   
        _delay_us (PATCHING);                       
        PIN_DX_INPUT;                      
        PIN_AX_INTERRUPT_DISABLE;          

        impulse = 0;                    
        patch = 1;                       // patch is set to 1, indicating that the first patch is completed.
      }
    }

    void Bios_Patching(){
            
      PIN_AX_INTERRUPT_FALLING;           

      if (PIN_AX_READ != 0)                 // If the AX pin is high
      {
        while (PIN_AX_READ != 0);           // Wait for it to go low
        while (PIN_AX_READ == 0);           // Then wait for it to go high again.
      }
      else                                  // If the AX pin is low
      {
        while (PIN_AX_READ == 0);           // Wait for it to go high.
      }
      
      _delay_ms(CHECKPOINT);        // Wait until the number of microseconds elapsed reaches a value defined by CHECKPOINT.
      PIN_AX_INTERRUPT_ENABLE;              
      
      while (patch != 1);                   // Wait for the first stage of the patch to complete:
    }

  #endif

  #ifdef INTERRUPT_RISING_HIGH_PATCH

    ISR(PIN_AX_INTERRUPT_VECTOR) {
      impulse++;                         
      if (impulse == TRIGGER){           // If impulse reaches the value defined by TRIGGER, the following actions are performed:
        _delay_us (HOLD);                                         
        PIN_DX_SET;                     
        PIN_DX_OUTPUT;                   
        _delay_us (PATCHING);                       
        PIN_DX_CLEAR;                     
        PIN_DX_INPUT;                      
        PIN_AX_INTERRUPT_DISABLE;          

        impulse = 0;                    
        patch = 1;                       // patch is set to 1, indicating that the first patch is completed.
      }
    }
  
    ISR(PIN_AY_INTERRUPT_VECTOR){

      impulse++;                         
      if (impulse == TRIGGER2)           // If impulse reaches the value defined by TRIGGER2, the following actions are performed:
      {
        _delay_us (HOLD2);                           
        PIN_DX_OUTPUT;                  
        _delay_us (PATCHING2);                      
        PIN_DX_INPUT;                        
        PIN_AY_INTERRUPT_DISABLE;           

        patch = 2;                       // patch is set to 2, indicating that the second patch is completed.
      }
    }

    void Bios_Patching(){

        PIN_AX_INTERRUPT_RISING;             

      if (PIN_AX_READ != 0)                 // If the AX pin is high
      {
        while (PIN_AX_READ != 0);           // Wait for it to go low
        while (PIN_AX_READ == 0);           // Then wait for it to go high again.
      }
      else                                  // If the AX pin is low
      {
        while (PIN_AX_READ == 0);           // Wait for it to go high.
      }
                          
      _delay_ms(CHECKPOINT);        // Wait until the number of microseconds elapsed reaches a value defined by CHECKPOINT.                  
      PIN_AX_INTERRUPT_ENABLE;              
      
      while (patch != 1);                   // Wait for the first stage of the patch to complete:


      #ifdef HIGH_PATCH              
        PIN_AY_INTERRUPT_FALLING;          
        #else
        PIN_AY_INTERRUPT_RISING;           
      #endif
    
      while (PIN_AY_READ != 0);             // Wait for it to go low    

      _delay_ms(CHECKPOINT2);     // Wait until the number of microseconds elapsed reaches a value defined by CHECKPOINT2.                      
      PIN_AY_INTERRUPT_ENABLE;            
      while (patch != 2);                 // Wait for the second stage of the patch to complete:

    }

  #endif




#endif


