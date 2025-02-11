/* ###################################################################
**     Filename    : main.c
**     Project     : ProbadorManillares
**     Processor   : MC9S08AC16CFD
**     Version     : Driver 01.12
**     Compiler    : CodeWarrior HCS08 C Compiler
**     Date/Time   : 2018-11-26, 12:56, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.12
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "Vtpm1ovf.h"
#include "Vadc1.h"
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

#include "main.h"

/* User includes (#include below this line is not maintained by Processor Expert) */





	/****************************************************************************************************************/
	/*  												VARIABLES													*/
	/****************************************************************************************************************/

// Variables para controlar los pulsadores
estadosTeclas 			estadoTeclaPresionada;
uint16_t				vecesSinRevisarTeclaAceptar;
uint16_t				vecesSinRevisarTeclaCancelar;
uint16_t				vecesSinRevisarTeclaDerecha;
uint16_t				vecesSinRevisarTeclaIzquierda;
char					pulsador;


// Variables para manejar los pernos
pernosObj				pernos[ TOTAL_DE_CONTACTOS_MULTIPLEXORES + 1 ];
resultadoPernos			estadoPernos;
resultadoPernos			condicionDelPerno[ TOTAL_DE_CONTACTOS_MULTIPLEXORES + 1 ];
char					pernosUbicacion[ TOTAL_DE_CONTACTOS_MULTIPLEXORES + 1 ];
bool 					hayPernoAbierto;
bool 					hayPernoEnCorto;
bool 					hayPernoEnCortoCon5V;
bool 					hayPernoEnCortoElMazo;
bool 					hayPernoAlternado;



// Variables para controlar el ADC
uint8_t					cuentasADC;
bool					semaforoADC;


// Variable generica para cualquier cosa
char 					auxiliar;


// Variable para indicar la finalizacion de los loops
bool					finalizarLoop;


// Variable para contemplar la devolucion de las funciones que indican algun error en su ejecucion
bool 					errorGeneral;



estadosPrograma			estadosDelPrograma = DEFINIENDO_1_CIFRA;

void ObtenerMatrizDePernos ( char target );
resultadoPernos VerificarMatrizObtenida ( char target );
void RenumerarContactos ( char target );


	/****************************************************************************************************************/
	/*  												PROGRAMA													*/
	/****************************************************************************************************************/

void main(void)
{
  /* Write your local variable definition here */ 
	bool	reimprimirPantallaInicial = TRUE;
	bool	reimprimirConfiguracion = TRUE;
	bool	iniciarPrueba = FALSE;
	bool	pruebaReducida = TRUE;
	char	indiceByte;
	char	opcionIncicadaEnElMenu = 1;
	
	char	primerCifra = 0;
	char	segundaCifra = 0;
	char	tercerCifra = 0;
	char	numeroElegido = 0;
	
	resultadoPernos	estadoDelCable;
	
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */

  
  
/* ******************** ******************** CODIGO PROPIO ******************** ******************** */

    // Se inicializan los puertos del micro
    InicializarPuertosProbador();
  
	// Se inicializa el timer
	InicializarTimer1();
	
	// Demora para dejar que se estabilice la fuente de alimentacion
	DemoraEnSegundos( 1 );
	
	// Se configura el tipo de LCD a utilizar
	errorGeneral = SeleccionarLCD( LCD_4_FILAS, LCD_20_CARACTERES, LCD_FUENTE_CHICA, LCD_4_BITS, LCD_PUERTO_EXTERNO, LCD_PROBADOR );
	
	// Se inicializa el LCD
	InicializarLCD();
	
	// Se configura el ADC
	ConfigurarADC();
	
	
	// Bucle perpetuo para realizar el programa
	while( TRUE )
	{
		// Se selecciona el estado del programa
		switch( estadosDelPrograma )
		{
			case DEFINIENDO_1_CIFRA:
				
				if( reimprimirConfiguracion == TRUE )
				{
					reimprimirConfiguracion = FALSE;
					// Se activa el cursor para que parpadeen las cifras cuando se las estan configurando
					enviarInstruccion( INSTRUCCION_LCD_PANTALLA + INSTRUCCION_LCD_PANTALLA_ON + INSTRUCCION_LCD_PANTALLA_CURSOR_OFF + INSTRUCCION_LCD_PANTALLA_BLINK_ON );
					demoraInicialLCD();
					BorrarLCD();
					errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, " <- Dec     -> Inc  " );
					errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       0, 20, " Ok Sig    Can Ant  " );
				}
				
				if( reimprimirPantallaInicial == TRUE )
				{
					// Se borra el indicador, porque ya se imprimio el mensaje inicial
					reimprimirPantallaInicial = FALSE;
					
					errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, " Elegir  MAZ000xxx  " );
					errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, " Definir 1ra cifra  " );
					EscribirCifra( primerCifra, RENGLON_SUPERIOR, 15 );
					
				}
				
				// Se espera la opresion de alguna tecla
				estadoTeclaPresionada = RevisarOpresionDeTeclas();
				
				// Se atiende la opresion de la tecla
				switch( estadoTeclaPresionada )
				{
					// Se debe decrementar la cifra
					case TECLA_IZQUIERDA_O_ARRIBA:
						
						// Si la cifra no es cero, se la decrementa
						if( primerCifra != 0 )
							primerCifra--;
						EscribirCifra( primerCifra, RENGLON_SUPERIOR, 15 );
						break;
						
					// Se debe incrementar la cifra
					case TECLA_DERECHA_O_ABAJO:
						
						// Si la cifra no alcanzo el limite superior, se la incrementa
						if( primerCifra < 1 )
							primerCifra++;
						EscribirCifra( primerCifra, RENGLON_SUPERIOR, 15 );
						break;
						
					// Se acepta la cifra y se pasa a la siguiente
					case TECLA_ACEPTAR:
						
						// Se cambia de estado para modificar la segunda cifra
						estadosDelPrograma = DEFINIENDO_2_CIFRA;
						reimprimirPantallaInicial = TRUE;
						break;
						
				}
				
				break;
				
			case DEFINIENDO_2_CIFRA:
				
				if( reimprimirPantallaInicial == TRUE )
				{
					// Se borra el indicador, porque ya se imprimio el mensaje inicial
					reimprimirPantallaInicial = FALSE;
					
					errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       16, 4, "xx  " );
					errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, " Definir 2da cifra  " );
					EscribirCifra( segundaCifra, RENGLON_SUPERIOR, 16 );
					
				}
				
				// Se espera la opresion de alguna tecla
				estadoTeclaPresionada = RevisarOpresionDeTeclas();
				
				// Se atiende la opresion de la tecla
				switch( estadoTeclaPresionada )
				{
					// Se debe decrementar la cifra
					case TECLA_IZQUIERDA_O_ARRIBA:
						
						// Si la cifra no es cero, se la decrementa
						if( segundaCifra != 0 )
							segundaCifra--;
						EscribirCifra( segundaCifra, RENGLON_SUPERIOR, 16 );
						break;
						
					// Se debe incrementar la cifra
					case TECLA_DERECHA_O_ABAJO:
						
						// Si la cifra no alcanzo el limite superior, se la incrementa
						if( segundaCifra < 9 )
							segundaCifra++;
						EscribirCifra( segundaCifra, RENGLON_SUPERIOR, 16 );
						break;
						
					// Se acepta la cifra y se pasa a la siguiente
					case TECLA_ACEPTAR:
						
						// Se cambia de estado para modificar la segunda cifra
						estadosDelPrograma = DEFINIENDO_3_CIFRA;
						reimprimirPantallaInicial = TRUE;
						break;
						
					// Se cancela la cifra y se pasa a la anterior
					case TECLA_CANCELAR:
						
						// Se cambia de estado para modificar la segunda cifra
						estadosDelPrograma = DEFINIENDO_1_CIFRA;
						reimprimirPantallaInicial = TRUE;
						break;
						
				}
				
				break;
				
			case DEFINIENDO_3_CIFRA:
				
				if( reimprimirPantallaInicial == TRUE )
				{
					// Se borra el indicador, porque ya se imprimio el mensaje inicial
					reimprimirPantallaInicial = FALSE;
					
					errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       17, 3, "x  " );
					errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, " Definir 3ra cifra  " );
					EscribirCifra( tercerCifra, RENGLON_SUPERIOR, 17 );
					
				}
				
				// Se espera la opresion de alguna tecla
				estadoTeclaPresionada = RevisarOpresionDeTeclas();
				
				// Se atiende la opresion de la tecla
				switch( estadoTeclaPresionada )
				{
					// Se debe decrementar la cifra
					case TECLA_IZQUIERDA_O_ARRIBA:
						
						// Si la cifra no es cero, se la decrementa
						if( tercerCifra != 0 )
							tercerCifra--;
						EscribirCifra( tercerCifra, RENGLON_SUPERIOR, 17 );
						break;
						
					// Se debe incrementar la cifra
					case TECLA_DERECHA_O_ABAJO:
						
						// Si la cifra no alcanzo el limite superior, se la incrementa
						if( tercerCifra < 9 )
							tercerCifra++;
						EscribirCifra( tercerCifra, RENGLON_SUPERIOR, 17 );
						break;
						
					// Se acepta la cifra y se pasa a la siguiente
					case TECLA_ACEPTAR:
						
						// Se cambia de estado para modificar la segunda cifra
						estadosDelPrograma = PROBANDO_CABLE;
						reimprimirPantallaInicial = TRUE;
						reimprimirConfiguracion = TRUE;
						numeroElegido = primerCifra * 100 + segundaCifra * 10 + tercerCifra;
						break;
						
					// Se cancela la cifra y se pasa a la anterior
					case TECLA_CANCELAR:
						
						// Se cambia de estado para modificar la segunda cifra
						estadosDelPrograma = DEFINIENDO_2_CIFRA;
						reimprimirPantallaInicial = TRUE;
						break;
						
				}
				
				break;
					
			// Una vez seleccionado el cable a probar, se ingresa en un loop para probar varios del mismo tipo
			case PROBANDO_CABLE:

				if( reimprimirConfiguracion == TRUE )
				{
					reimprimirConfiguracion = FALSE;
					
					// Se desactiva el cursor para que dejen de parpadear
					enviarInstruccion( INSTRUCCION_LCD_PANTALLA + INSTRUCCION_LCD_PANTALLA_ON + INSTRUCCION_LCD_PANTALLA_CURSOR_OFF + INSTRUCCION_LCD_PANTALLA_BLINK_OFF );
					demoraInicialLCD();
					BorrarLCD();
					errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "      MAZ000xxx     " );
					EscribirNumeroDeCable( numeroElegido, RENGLON_SUPERIOR, 12 );
					errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, " Ok: Probar cable   " );
					errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       0, 20, " Cancelar: Volver   " );
				}
				
				if( reimprimirPantallaInicial == TRUE )
				{
					
				}
				
				// Se espera la opresion de alguna tecla
				estadoTeclaPresionada = RevisarOpresionDeTeclas();
				
				// Se atiende la opresion de la tecla
				switch( estadoTeclaPresionada )
				{
					// Se debe probar el cable elegido 
					case TECLA_ACEPTAR:
						errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR,      0, 20, "                    " );
						
						// Se obtiene la matriz y se verifica la misma segun el cable seleccionado
						switch( numeroElegido )
						{
							// Lugar 1
							case 1:
								ObtenerMatrizDePernos( CABLE_MAZO_001 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_001 );
								RenumerarContactos( CABLE_MAZO_001 );
								break;
							case 41:
								ObtenerMatrizDePernos( CABLE_MAZO_041 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_041 );
								RenumerarContactos( CABLE_MAZO_041 );
								break;
							case 105:
								ObtenerMatrizDePernos( CABLE_MAZO_105 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_105 );
								RenumerarContactos( CABLE_MAZO_105 );
								break;
							
							// Lugar 2
							case 2:
								ObtenerMatrizDePernos( CABLE_MAZO_002 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_002 );
								RenumerarContactos( CABLE_MAZO_002 );
								break;
							case 42:
								ObtenerMatrizDePernos( CABLE_MAZO_042 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_042 );
								RenumerarContactos( CABLE_MAZO_042 );
								break;
							case 72:
								ObtenerMatrizDePernos( CABLE_MAZO_072 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_072 );
								RenumerarContactos( CABLE_MAZO_072 );
								break;
							case 106:
								ObtenerMatrizDePernos( CABLE_MAZO_106 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_106 );
								RenumerarContactos( CABLE_MAZO_106 );
								break;
								
							// Lugar 3
							case 3:
								ObtenerMatrizDePernos( CABLE_MAZO_003 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_003 );
								RenumerarContactos( CABLE_MAZO_003 );
								break;
							case 4:
								ObtenerMatrizDePernos( CABLE_MAZO_004 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_004 );
								RenumerarContactos( CABLE_MAZO_004 );
								break;
							case 5:
								ObtenerMatrizDePernos( CABLE_MAZO_005 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_005 );
								RenumerarContactos( CABLE_MAZO_005 );
								break;
							case 26:
								ObtenerMatrizDePernos( CABLE_MAZO_026 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_026 );
								RenumerarContactos( CABLE_MAZO_026 );
								break;
							case 43:
								ObtenerMatrizDePernos( CABLE_MAZO_043 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_043 );
								RenumerarContactos( CABLE_MAZO_043 );
								break;
							case 44:
								ObtenerMatrizDePernos( CABLE_MAZO_044 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_044 );
								RenumerarContactos( CABLE_MAZO_044 );
								break;
							case 60:
								ObtenerMatrizDePernos( CABLE_MAZO_060 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_060 );
								RenumerarContactos( CABLE_MAZO_060 );
								break;
							case 73:
								ObtenerMatrizDePernos( CABLE_MAZO_073 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_073 );
								RenumerarContactos( CABLE_MAZO_073 );
								break;
							case 74:
								ObtenerMatrizDePernos( CABLE_MAZO_074 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_074 );
								RenumerarContactos( CABLE_MAZO_074 );
								break;
							case 110:
								ObtenerMatrizDePernos( CABLE_MAZO_110 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_110 );
								RenumerarContactos( CABLE_MAZO_110 );
								break;
								
							// Lugar 4
							case 6:
								ObtenerMatrizDePernos( CABLE_MAZO_006 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_006 );
								RenumerarContactos( CABLE_MAZO_006 );
								break;
							case 7:
								ObtenerMatrizDePernos( CABLE_MAZO_007 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_007 );
								RenumerarContactos( CABLE_MAZO_007 );
								break;
							case 45:
								ObtenerMatrizDePernos( CABLE_MAZO_045 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_045 );
								RenumerarContactos( CABLE_MAZO_045 );
								break;
							case 46:
								ObtenerMatrizDePernos( CABLE_MAZO_046 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_046 );
								RenumerarContactos( CABLE_MAZO_046 );
								break;
							case 75:
								ObtenerMatrizDePernos( CABLE_MAZO_075 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_075 );
								RenumerarContactos( CABLE_MAZO_075 );
								break;
							case 107:
								ObtenerMatrizDePernos( CABLE_MAZO_107 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_107 );
								RenumerarContactos( CABLE_MAZO_107 );
								break;
								
							// Lugar 5
							case 12:
								ObtenerMatrizDePernos( CABLE_MAZO_012 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_012 );
								RenumerarContactos( CABLE_MAZO_012 );
								break;
							case 51:
								ObtenerMatrizDePernos( CABLE_MAZO_051 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_051 );
								RenumerarContactos( CABLE_MAZO_051 );
								break;
								
							// Lugar 6
							case 25:
								ObtenerMatrizDePernos( CABLE_MAZO_025 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_025 );
								RenumerarContactos( CABLE_MAZO_025 );
								break;
							case 71:
								ObtenerMatrizDePernos( CABLE_MAZO_071 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_071 );
								RenumerarContactos( CABLE_MAZO_071 );
								break;
							case 83:
								ObtenerMatrizDePernos( CABLE_MAZO_083 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_083 );
								RenumerarContactos( CABLE_MAZO_083 );
								break;
								
							// Lugar 7
							case 47:
								ObtenerMatrizDePernos( CABLE_MAZO_047 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_047 );
								RenumerarContactos( CABLE_MAZO_047 );
								break;
							case 76:
								ObtenerMatrizDePernos( CABLE_MAZO_076 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_076 );
								RenumerarContactos( CABLE_MAZO_076 );
								break;
									
							// Lugar 8
							case 11:
								ObtenerMatrizDePernos( CABLE_MAZO_011 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_011 );
								RenumerarContactos( CABLE_MAZO_011 );
								break;
							case 50:
								ObtenerMatrizDePernos( CABLE_MAZO_050 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_050 );
								RenumerarContactos( CABLE_MAZO_050 );
								break;
							case 108:
								ObtenerMatrizDePernos( CABLE_MAZO_108 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_108 );
								RenumerarContactos( CABLE_MAZO_108 );
								break;
								
							// Lugar 9
							case 23:
								ObtenerMatrizDePernos( CABLE_MAZO_023 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_023 );
								RenumerarContactos( CABLE_MAZO_023 );
								break;
							case 81:
								ObtenerMatrizDePernos( CABLE_MAZO_081 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_081 );
								RenumerarContactos( CABLE_MAZO_081 );
								break;
								
							// Lugar 10
							case 21:
								ObtenerMatrizDePernos( CABLE_MAZO_021 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_021 );
								RenumerarContactos( CABLE_MAZO_021 );
								break;
							case 79:
								ObtenerMatrizDePernos( CABLE_MAZO_079 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_079 );
								RenumerarContactos( CABLE_MAZO_079 );
								break;
								
							// Lugar 11
							case 31:
								ObtenerMatrizDePernos( CABLE_MAZO_031 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_031 );
								RenumerarContactos( CABLE_MAZO_031 );
								break;
								
							// Lugar 11
							case 38:
								ObtenerMatrizDePernos( CABLE_MAZO_038 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_038 );
								RenumerarContactos( CABLE_MAZO_038 );
								break;
									
							// Lugar 13
							case 78:
								ObtenerMatrizDePernos( CABLE_MAZO_078 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_078 );
								RenumerarContactos( CABLE_MAZO_078 );
								break;

							// Lugar 14
							case 30:
								ObtenerMatrizDePernos( CABLE_MAZO_030 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_030 );
								RenumerarContactos( CABLE_MAZO_030 );
								break;
							case 64:
								ObtenerMatrizDePernos( CABLE_MAZO_030 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_030 );
								RenumerarContactos( CABLE_MAZO_030 );
								break;
								
							// Lugar 15
							case 90:
								ObtenerMatrizDePernos( CABLE_MAZO_090 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_090 );
								RenumerarContactos( CABLE_MAZO_090 );
								break;

							// Lugar 16
							case 9:
								ObtenerMatrizDePernos( CABLE_MAZO_009 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_009 );
								RenumerarContactos( CABLE_MAZO_009 );
								break;
							case 48:
								ObtenerMatrizDePernos( CABLE_MAZO_048 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_048 );
								RenumerarContactos( CABLE_MAZO_048 );
								break;
							case 77:
								ObtenerMatrizDePernos( CABLE_MAZO_077 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_077 );
								RenumerarContactos( CABLE_MAZO_077 );
								break;
								
							// Lugar 17
							case 22:
								ObtenerMatrizDePernos( CABLE_MAZO_022 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_022 );
								RenumerarContactos( CABLE_MAZO_022 );
								break;

							// Lugar 18
							case 24:
								ObtenerMatrizDePernos( CABLE_MAZO_024 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_024 );
								RenumerarContactos( CABLE_MAZO_024 );
								break;
							case 109:
								ObtenerMatrizDePernos( CABLE_MAZO_109 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_109 );
								RenumerarContactos( CABLE_MAZO_109 );
								break;
								
							// Lugar 19
							case 82:
								ObtenerMatrizDePernos( CABLE_MAZO_082 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_082 );
								RenumerarContactos( CABLE_MAZO_082 );
								break;
								
							// Lugar 20
							case 96:
								ObtenerMatrizDePernos( CABLE_MAZO_096 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_096 );
								RenumerarContactos( CABLE_MAZO_096 );
								break;
								
							// Lugar 21 - El MAZ000097 se debe probar en 2 partes
							case 97:
								// Parte A - Conector CPC de 14 pines + Conector SZCNT monopoloar contra los conectores a las placas
								errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "MAZ000097A - Parte A" );
								ObtenerMatrizDePernos( CABLE_MAZO_097_A );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_097_A );
								RenumerarContactos( CABLE_MAZO_097_A );
								// Se indica el estado del cable en el display
								switch( estadoDelCable )
								{
									case PERNO_SIN_ERRORES:
										errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR,      0, 20, "      Cable OK      " );
										break;
									case PERNO_ABIERTO:				InformarPernosAbiertos();	break;
									case PERNO_CORTO_CIRCUITO:		InformarPernosEnCorto();	break;
									case PERNO_SIN_ALTERNANCIA:		InformarMalUbicacion();		break;
								}
								// Se genera una demora para que se pueda leer el mensaje
								DemoraEnSegundos( 4 );
								
								// Parte B - Conector SZCNT de 7 pines contra los conectores a las placas
								errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       		0, 20, "MAZ000097A - Parte B" );
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR,      0, 20, "                    " );
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 		0, 20, " Ok: Probar cable   " );
								errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       		0, 20, " Cancelar: Volver   " );
								// Se espera la opresion de la tecla aceptar para continuar
								EsperarOpresionDeTecla( TECLA_ACEPTAR );
								
								ObtenerMatrizDePernos( CABLE_MAZO_097_B );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_097_B );
								RenumerarContactos( CABLE_MAZO_097_B );
								break;
								
							case 98:
								// Parte A - Conector CPC de 14 pines + Conector SZCNT monopoloar contra los conectores a las placas
								errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "MAZ000098A - Parte A" );
								ObtenerMatrizDePernos( CABLE_MAZO_097_A );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_097_A );
								RenumerarContactos( CABLE_MAZO_097_A );
								// Se indica el estado del cable en el display
								switch( estadoDelCable )
								{
									case PERNO_SIN_ERRORES:
										errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR,      0, 20, "      Cable OK      " );
										break;
									case PERNO_ABIERTO:				InformarPernosAbiertos();	break;
									case PERNO_CORTO_CIRCUITO:		InformarPernosEnCorto();	break;
									case PERNO_SIN_ALTERNANCIA:		InformarMalUbicacion();		break;
								}
								// Se genera una demora para que se pueda leer el mensaje
								DemoraEnSegundos( 4 );
								
								// Parte B - Conector SZCNT de 7 pines contra los conectores a las placas
								errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       		0, 20, "MAZ000098A - Parte B" );
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR,      0, 20, "                    " );
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 		0, 20, " Ok: Probar cable   " );
								errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       		0, 20, " Cancelar: Volver   " );
								// Se espera la opresion de la tecla aceptar para continuar
								EsperarOpresionDeTecla( TECLA_ACEPTAR );
								
								ObtenerMatrizDePernos( CABLE_MAZO_097_B );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_097_B );
								RenumerarContactos( CABLE_MAZO_097_B );
								
								break;
								
							// Lugar 23
							case 32:
								ObtenerMatrizDePernos( CABLE_MAZO_032 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_032 );
								RenumerarContactos( CABLE_MAZO_032 );
								break;
							case 66:
								ObtenerMatrizDePernos( CABLE_MAZO_066 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_066 );
								RenumerarContactos( CABLE_MAZO_066 );
								break;
								
							// Lugar 24
							case 111:
								ObtenerMatrizDePernos( CABLE_MAZO_111 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_111 );
								RenumerarContactos( CABLE_MAZO_111 );
								break;
								
							// Lugar 25
							case 35:
								ObtenerMatrizDePernos( CABLE_MAZO_035 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_035 );
								RenumerarContactos( CABLE_MAZO_035 );
								break;
								
							// Lugar 26
							case 34:
								ObtenerMatrizDePernos( CABLE_MAZO_034 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_034 );
								RenumerarContactos( CABLE_MAZO_034 );
								break;
								
							// Lugar para el cable de telefono
							default:
								ObtenerMatrizDePernos( CABLE_MAZO_000 );
								estadoDelCable = VerificarMatrizObtenida( CABLE_MAZO_000 );
								RenumerarContactos( CABLE_MAZO_000 );
								break;
						}
						
						// Se indica el estado del cable en el display
						switch( estadoDelCable )
						{
							case PERNO_SIN_ERRORES:
								errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR,      0, 20, "      Cable OK      " );
								break;
							case PERNO_ABIERTO:				InformarPernosAbiertos();	break;
							case PERNO_CORTO_CIRCUITO:		InformarPernosEnCorto();	break;
							case PERNO_SIN_ALTERNANCIA:		InformarMalUbicacion();		break;
						}
						
						// Se genera una demora para que se pueda leer el mensaje
						DemoraEnSegundos( 4 );
						reimprimirConfiguracion = TRUE;
						break;
						
					// Se debe volver al menu anterior
					case TECLA_CANCELAR:
						reimprimirConfiguracion = TRUE;
						reimprimirPantallaInicial = TRUE;
						estadosDelPrograma = DEFINIENDO_1_CIFRA;
						break;
				}
				
				break;
				
		}
		
	}
	
	// Por las dudas que falle algo, se coloca un loop para dejar el programa en estado de bajo consumo
	while( TRUE )	{ wait(); }
	
  /* ******************** CODIGO PROPIO ******************** */
  
  /* For example: for(;;) { } */

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/


/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.3 [05.09]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/





	/****************************************************************************************************************/
	/*												FUNCIONES GENERALES												*/
	/****************************************************************************************************************/


/****************************************************************/
/* wait															*/
/*  															*/
/*  Coloca el micro controlador en estado de espera, del cual	*/
/*  se repone mediante una interrupcion. Sirve para evitar un	*/
/*  consumo innecesario durante las demoras.					*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void wait ( void )
{
	asm(wait);
}



/****************************************************************/
/* EsperarOpresionDeTecla										*/
/*  															*/
/*  Genera una demora bloqueante hasta que se oprima la tecla	*/
/*  solicitada													*/
/*  															*/
/*  Recibe: El pulsador que se quiere esperar					*/
/*  Devuelve: Nada												*/
/****************************************************************/
void EsperarOpresionDeTecla ( char tecla )
{
	switch( tecla )
	{
		case TECLA_DERECHA_O_ABAJO:
			while( TRUE )
			{
				pulsador = getRegBits( PULSADOR_S3_PUERTO, PULSADOR_S3_MASCARA );
				if( pulsador == 0 )
				{
					// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
					for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
					{
						DemoraPorInterrupcion( MILISEGUNDOS_10 );
						pulsador = getRegBits( PULSADOR_S3_PUERTO, PULSADOR_S3_MASCARA );
						if( pulsador > 0 )
							break;
					}
					
					// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
					if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
						// Se rompe el while
						break;
				}
				
				DemoraPorInterrupcion( MILISEGUNDOS_50 );
			}
			break;
			
		case TECLA_IZQUIERDA_O_ARRIBA:
			while( TRUE )
			{
				pulsador = getRegBits( PULSADOR_S2_PUERTO, PULSADOR_S2_MASCARA );
				if( pulsador == 0 )
				{
					// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
					for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
					{
						DemoraPorInterrupcion( MILISEGUNDOS_10 );
						pulsador = getRegBits( PULSADOR_S2_PUERTO, PULSADOR_S2_MASCARA );
						if( pulsador > 0 )
							break;
					}
					
					// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
					if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
						// Se rompe el while
						break;
				}
				
				DemoraPorInterrupcion( MILISEGUNDOS_50 );
			}
			break;
			
			
		case TECLA_ACEPTAR:
			while( TRUE )
			{
				pulsador = getRegBits( PULSADOR_S1_PUERTO, PULSADOR_S1_MASCARA );
				if( pulsador == 0 )
				{
					// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
					for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
					{
						DemoraPorInterrupcion( MILISEGUNDOS_10 );
						pulsador = getRegBits( PULSADOR_S1_PUERTO, PULSADOR_S1_MASCARA );
						if( pulsador > 0 )
							break;
					}
					
					// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
					if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
						// Se rompe el while
						break;
				}
				
				DemoraPorInterrupcion( MILISEGUNDOS_50 );
			}
			break;
	}
}



/****************************************************************/
/* RevisarOpresionDeTeclas										*/
/*  															*/
/*  Revisa el estado de los pulsdaores y devuelve un indicador  */
/*  de cual se oprimio											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: El pulsador que se oprimio						*/
/****************************************************************/
estadosTeclas RevisarOpresionDeTeclas ( void )
{
	// Bucle perpetuo hasta esperar la opresion de algun pulsador
	while( TRUE )
	{
		
		// Se revisa el pulsador derecho
		pulsador = getRegBits( PULSADOR_S3_PUERTO, PULSADOR_S3_MASCARA );
		if( pulsador == 0 )
		{
			// Se revisa que haya pasado cierto tiempo desde la ultima opresion, antes de tomarlo como una nueva
			if( vecesSinRevisarTeclaDerecha > MINIMO_DE_VECES_SIN_REVISAR_OPRESION )
			{
				// Se borra el indicador
				vecesSinRevisarTeclaDerecha = 0;
				
				// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
				for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
				{
					DemoraPorInterrupcion( MILISEGUNDOS_10 );
					pulsador = getRegBits( PULSADOR_S3_PUERTO, PULSADOR_S3_MASCARA );
					if( pulsador > 0 )
						break;
				}
				
				// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
				if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
					return( TECLA_DERECHA_O_ABAJO );
			}
		}
		
		
		// Se revisa el pulsador izquierdo
		pulsador = getRegBits( PULSADOR_S2_PUERTO, PULSADOR_S2_MASCARA );
		if( pulsador == 0 )
		{
			// Se revisa que haya pasado cierto tiempo desde la ultima opresion, antes de tomarlo como una nueva
			if( vecesSinRevisarTeclaIzquierda > MINIMO_DE_VECES_SIN_REVISAR_OPRESION )
			{
				// Se borra el indicador
				vecesSinRevisarTeclaIzquierda = 0;
				
				// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
				for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
				{
					DemoraPorInterrupcion( MILISEGUNDOS_10 );
					pulsador = getRegBits( PULSADOR_S2_PUERTO, PULSADOR_S2_MASCARA );
					if( pulsador > 0 )
						break;
				}
				
				// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
				if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
					return( TECLA_IZQUIERDA_O_ARRIBA );
			}
		}
		
		
		// Se revisa el pulsador de aceptar
		pulsador = getRegBits( PULSADOR_S1_PUERTO, PULSADOR_S1_MASCARA );
		if( pulsador == 0 )
		{
			// Se revisa que haya pasado cierto tiempo desde la ultima opresion, antes de tomarlo como una nueva
			if( vecesSinRevisarTeclaAceptar > MINIMO_DE_VECES_SIN_REVISAR_OPRESION )
			{
				// Se borra el indicador
				vecesSinRevisarTeclaAceptar = 0;
				
				// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
				for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
				{
					DemoraPorInterrupcion( MILISEGUNDOS_10 );
					pulsador = getRegBits( PULSADOR_S1_PUERTO, PULSADOR_S1_MASCARA );
					if( pulsador > 0 )
						break;
				}
				
				// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
				if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
					return( TECLA_ACEPTAR );
			}
		}
		
		
		// Se revisa el pulsador de cancelar
		pulsador = getRegBits( PULSADOR_S4_PUERTO, PULSADOR_S4_MASCARA );
		if( pulsador == 0 )
		{
			// Se revisa que haya pasado cierto tiempo desde la ultima opresion, antes de tomarlo como una nueva
			if( vecesSinRevisarTeclaCancelar > MINIMO_DE_VECES_SIN_REVISAR_OPRESION )
			{
				// Se borra el indicador
				vecesSinRevisarTeclaCancelar = 0;
				
				// Si se detecta una opresion, se revisa que se mantenga durante cierto tiempo para darla por valida
				for( auxiliar = 0; auxiliar < MINIMO_DE_OPRESIONES_CUALQUIER_TECLA; auxiliar++ )
				{
					DemoraPorInterrupcion( MILISEGUNDOS_10 );
					pulsador = getRegBits( PULSADOR_S4_PUERTO, PULSADOR_S4_MASCARA );
					if( pulsador > 0 )
						break;
				}
				
				// Si el contador llego al maximo, la tecla se mantuvo presionada durante un tiempo valido
				if( auxiliar == MINIMO_DE_OPRESIONES_CUALQUIER_TECLA )
					return( TECLA_CANCELAR );
				}
		}
		
		// Demora hasta revisar nuevamente los pulsadores 
		DemoraPorInterrupcion( MILISEGUNDOS_50 );
		
		// Se incrementan los contadores
		vecesSinRevisarTeclaAceptar++;
		vecesSinRevisarTeclaCancelar++;
		vecesSinRevisarTeclaDerecha++;
		vecesSinRevisarTeclaIzquierda++;
		
	}
}



// Funciones para prender o apagar los leds rojo y verde de la placa, que se usan para indicar el estado de las pruebas
void ApagarLedVerde ( void )	{	LED_VERDE_PIN = 0;	}
void ApagarLedRojo ( void )		{	LED_ROJO_PIN = 0;	}
void EncenderLedVerde ( void )	{	LED_VERDE_PIN = 1;	}
void EncenderLedRojo ( void )	{	LED_ROJO_PIN = 1;	}
void IndicarPruebaBien ( void )	{ EncenderLedVerde(); ApagarLedRojo(); }
void IndicarPruebaMal ( void )	{ ApagarLedVerde(); EncenderLedRojo(); }
void ApagarLeds ( void )		{ ApagarLedVerde(); ApagarLedRojo(); }


void AlternarLedRojo( void )
{
	if( LED_ROJO_PIN == 1 )
		ApagarLedRojo();
	else
		EncenderLedRojo();
}

void AlternarLedVerde( void )
{
	if( LED_VERDE_PIN == 1 )
		ApagarLedVerde();
	else
		EncenderLedVerde();
}




	/****************************************************************************************************************/
	/*  								FIRMWARE PARA LA PRUEBA REDUCIDA DEL MANILLAR								*/
	/****************************************************************************************************************/



/****************************************************************/
/* InicializarPuertosProbador									*/
/*  															*/
/*  Define la funcion que tendra cada pin del microcontrolador	*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InicializarPuertosProbador( void )
{
	/* PINES PARA LOS MULTIPLEXORES */
	
	// Pines para el multiplexor de los contactos de la placa
	S0_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	S1_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	S2_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	S3_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	E_MULTIPLEXOR_PLACA_DIRECCION = 1;						// Salida
	S0_MULTIPLEXOR_PLACA_PIN = 0;							// Activo nivel alto
	S1_MULTIPLEXOR_PLACA_PIN = 0;							// Activo nivel alto
	S2_MULTIPLEXOR_PLACA_PIN = 0;							// Activo nivel alto
	S3_MULTIPLEXOR_PLACA_PIN = 0;							// Activo nivel alto
	E_MULTIPLEXOR_PLACA_PIN = 1;							// Activo nivel bajo
	
	// Pines para el multiplexor de los contactos desde los pernos
	S0_MULTIPLEXOR_PERNOS_DIRECCION = 1;					// Salida
	S1_MULTIPLEXOR_PERNOS_DIRECCION = 1;					// Salida
	S2_MULTIPLEXOR_PERNOS_DIRECCION = 1;					// Salida
	S3_MULTIPLEXOR_PERNOS_DIRECCION = 1;					// Salida
	E_MULTIPLEXOR_PERNOS_DIRECCION = 1;						// Salida
	S0_MULTIPLEXOR_PERNOS_PIN = 0;							// Activo nivel alto
	S1_MULTIPLEXOR_PERNOS_PIN = 0;							// Activo nivel alto
	S2_MULTIPLEXOR_PERNOS_PIN = 0;							// Activo nivel alto
	S3_MULTIPLEXOR_PERNOS_PIN = 0;							// Activo nivel alto
	E_MULTIPLEXOR_PERNOS_PIN = 1;							// Activo nivel bajo
	
	// Se define la salida del multiplexor donde se mide la continuidad como entrada. Luego se usaran las funciones analogicas
	PERNOS_SALIDA_CONTINUIDAD_DIRECCION = 0;				// Entrada
	
	/* PINES PARA CONTROLAR LOS LEDS */
	LED_ROJO_DIRECCION = 1;									// Salida
	LED_ROJO_PIN = 0;										// Activo nivel alto
	LED_VERDE_DIRECCION = 1;								// Salida
	LED_VERDE_PIN = 0;										// Activo nivel alto
	
	/* PINES PARA CONTROLAR LOS PULSADORES */
	PULSADOR_S1_DIRECCION = 0;								// Entrada
	PULSADOR_S2_DIRECCION = 0;								// Entrada
	PULSADOR_S3_DIRECCION = 0;								// Entrada
	PULSADOR_S4_DIRECCION = 0;								// Entrada
	
}





		/****************************************************************************************************************/
		/*				  								FUNCIONES PARA EL ADC											*/
		/****************************************************************************************************************/


/****************************************************************/
/* ConfigurarADC												*/
/*  															*/
/*  Setea el modulo para usarlo en baja velocidad, con el bus	*/
/*  principal dividido 16 (8x2), tiempos largos para la toma de	*/
/*	muestras y disparado por software							*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void ConfigurarADC ( void )
{
	ADC1CFG = 0xF1;											// Baja velocidad, dividido 8, tiempos largos, 8 bits y dividido 2
	ADC1SC2 = 0;											// Disparable por software, sin comparacion
	APCTL1 = PIN_ADC_PERNOS_CONTINUIDAD | PIN_ADC_LEDS;		// Se habilitan los pines de los leds del manillar y la Peltier
	APCTL2 = PIN_ADC_PELTIER;								// Se habilita el pin del canal para las salidas de los pernos
}



/****************************************************************/
/* TomarMuestraSimpleADC										*/
/*  															*/
/*  Se inicia una conversion simple en el canal seleccionado y  */
/*  se espera por la interrupcion de la conversion.				*/
/*	El resultado se almacena en la variable global "cuentasADC"	*/
/*  															*/
/*  Recibe: El canal del cual se deba leer el valor				*/
/*  Devuelve: FALSE, si hay algun problema						*/
/****************************************************************/
bool TomarMuestraSimpleADC( char canal )
{
	// Primero se verifica que el canal suministrado sea valido y se inicia la conversion
	if( iniciarADC_Simple( canal ) == FALSE )
		return( FALSE );
	
	// Se borra el flag de la interrupcion
	semaforoADC = FALSE;
	
	// Luego se espera a que termine la conversion
	while( TRUE )
	{
		if( semaforoADC == TRUE )
		{
			semaforoADC = FALSE;
			break;
		}
	}
	
	// Se toma el valor de la conversion
	cuentasADC = leerADC();
	
	return( TRUE );
}



// Iniciar una conversion simple
bool iniciarADC_Simple ( uint8_t canal )
{
	switch(canal)
	{
		case ADC_CANAL_LEDS:
			ADC1SC1 = 0x40 + ADC_CANAL_LEDS;
			break;
		case ADC_CANAL_PELTIER:
			ADC1SC1 = 0x40 + ADC_CANAL_PELTIER;
			break;
		case ADC_CANAL_PERNOS:
			ADC1SC1 = 0x40 + ADC_CANAL_PERNOS;
			break;
		default:
			return( FALSE );	
	}
	
	// Si el canal era correcto, se devuelve TRUE 
	return( TRUE );
}

// Se devuelve el registro del ADC donde se almacena el resultado
uint8_t leerADC ( void )				{	return( ADC1RL );	}

// Funcion para borrar el flag de la interrupcion
void borrarFlagADC ( void )				{	clrReg8Bits(ADC1SC1, 0x80U);	}





		/****************************************************************************************************************/
		/*		  								FUNCIONES PARA INFORMAR LOS ERRORES										*/
		/****************************************************************************************************************/


/****************************************************************/
/* InformarPernosAbiertos										*/
/*  															*/
/*  Muestra en pantalla la indicacion de que pernos se			*/
/*  encuentran abiertos											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void	InformarPernosAbiertos ( void )
{
	// Variable auxiliar
	char	indiceCable;
	char	offset;
	char	indiceCableCaracter[3];
	
	// Se informa que hay un error por que algunos pines estan abiertos
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 	0, 20, "Pines abiertos      " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 	0, 20, "                    " );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 		0, 20, "                    " );
	
	indiceCableCaracter[2] = '-';
	indiceCableCaracter[0] = '0';
	
	
	// Cables del 1 al 2
	for( indiceCable = 1; indiceCable <= 2; indiceCable++ )
	{
		if( condicionDelPerno[ indiceCable ] == PERNO_ABIERTO )
		{
			offset = 12 + indiceCable * 3;
			indiceCableCaracter[1] = indiceCable + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, offset, 3, indiceCableCaracter );
		}
	}
	
	// Cables del 3 al 9
	for( indiceCable = 3; indiceCable <= 9; indiceCable++ )
	{
		if( condicionDelPerno[ indiceCable ] == PERNO_ABIERTO )
		{
			offset = ( indiceCable - 3 ) * 3;
			indiceCableCaracter[1] = indiceCable + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, offset, 3, indiceCableCaracter );
		}
	}
	
	
	indiceCableCaracter[0] = '1';
	
	// Cables del 10 al 16
	for( indiceCable = 10; indiceCable <= 16; indiceCable++ )
	{
		if( condicionDelPerno[ indiceCable ] == PERNO_ABIERTO )
		{
			offset = ( indiceCable - 10 ) * 3;
			indiceCableCaracter[1] = indiceCable - 10 + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, offset, 3, indiceCableCaracter );
		}
	}
	
}



/****************************************************************/
/* InformarPernosEnCorto										*/
/*  															*/
/*  Muestra en pantalla la indicacion de que pernos se			*/
/*  encuentran en corto circuito entre si						*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void	InformarPernosEnCorto ( void )
{
	// Variable auxiliar
	char	indiceCable;
	char	offset;
	char	indiceCableCaracter[3];
	
	// Se informa que hay un error por que algunos pines estan abiertos
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 	0, 20, "Pines en corto      " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 	0, 20, "                    " );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 		0, 20, "                    " );
	
	indiceCableCaracter[2] = '-';
	indiceCableCaracter[0] = '0';
	
	
	// Cables del 1 al 2
	for( indiceCable = 1; indiceCable <= 2; indiceCable++ )
	{
		if( condicionDelPerno[ indiceCable ] == PERNO_CORTO_CIRCUITO )
		{
			offset = 12 + indiceCable * 3;
			indiceCableCaracter[1] = indiceCable + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, offset, 3, indiceCableCaracter );
		}
	}
	
	// Cables del 3 al 9
	for( indiceCable = 3; indiceCable <= 9; indiceCable++ )
	{
		if( condicionDelPerno[ indiceCable ] == PERNO_CORTO_CIRCUITO )
		{
			offset = ( indiceCable - 3 ) * 3;
			indiceCableCaracter[1] = indiceCable + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, offset, 3, indiceCableCaracter );
		}
	}
	
	
	indiceCableCaracter[0] = '1';
	
	// Cables del 10 al 16
	for( indiceCable = 10; indiceCable <= 16; indiceCable++ )
	{
		if( condicionDelPerno[ indiceCable ] == PERNO_CORTO_CIRCUITO )
		{
			offset = ( indiceCable - 10 ) * 3;
			indiceCableCaracter[1] = indiceCable - 10 + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, offset, 3, indiceCableCaracter );
		}
	}
	
}



/****************************************************************/
/* InformarMalUbicacion											*/
/*  															*/
/*  Muestra en pantalla la indicacion de que pernos se			*/
/*  encuentran en una posicion incorrecta						*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InformarMalUbicacion ( void )
{
	// Variable auxiliar
	char	indiceCable;
	char	offset;
	char	indiceCableCaracter[3];
	
	// Se informa que hay un error por que algunos pines estan abiertos
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 	0, 20, "Alternados          " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 	0, 20, "                    " );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 		0, 20, "                    " );
	
	indiceCableCaracter[2] = '-';
	indiceCableCaracter[0] = '0';
	
	
	// Cables del 1 al 2
	for( indiceCable = 1; indiceCable <= 2; indiceCable++ )
	{
		if( condicionDelPerno[ indiceCable ] == PERNO_SIN_ALTERNANCIA )
		{
			offset = 12 + indiceCable * 3;
			indiceCableCaracter[1] = indiceCable + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, offset, 3, indiceCableCaracter );
		}
	}
	
	// Cables del 3 al 9
	for( indiceCable = 3; indiceCable <= 9; indiceCable++ )
	{
		if( condicionDelPerno[ indiceCable ] == PERNO_SIN_ALTERNANCIA )
		{
			offset = ( indiceCable - 3 ) * 3;
			indiceCableCaracter[1] = indiceCable + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, offset, 3, indiceCableCaracter );
		}
	}
	
	
	indiceCableCaracter[0] = '1';
	
	// Cables del 10 al 16
	for( indiceCable = 10; indiceCable <= 16; indiceCable++ )
	{
		if( condicionDelPerno[ indiceCable ] == PERNO_SIN_ALTERNANCIA )
		{
			offset = ( indiceCable - 10 ) * 3;
			indiceCableCaracter[1] = indiceCable - 10 + '0';
			errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, offset, 3, indiceCableCaracter );
		}
	}
}








		/****************************************************************************************************************/
		/*		  						FUNCIONES PARA MOSTRAR LAS PANTALLAS DE LOS MENUES								*/
		/****************************************************************************************************************/



/****************************************************************/
/* ProbarCables													*/
/*  															*/
/*  Ejecuta la secuencia de prueba de continuidad de todos los  */
/*  contactos del MAZO66A										*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: El estado de los cables							*/
/****************************************************************/
resultadoPernos ProbarCables ( void )
{/*
	// Variables auxiliares
	char indiceContactoPernos;
	char indiceContactoPlaca;
	char direccionContactoPernos;
	char direccionContactoPlaca;
	bool hayPernoAbierto;
	bool hayPernoEnCorto;
	bool hayPernoAlternado;
	
	// Se borran los contadores
	hayPernoAbierto = FALSE;
	hayPernoEnCorto = FALSE;
	hayPernoAlternado = FALSE;
	
	// Se borran los indicadores
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_CONTACTOS_MAZO_66A + 1; indiceContactoPlaca++ )
	{
		condicionDelPerno[ indiceContactoPlaca ] = PERNO_SIN_ERRORES;
		pernosUbicacion[ indiceContactoPlaca ] = 0;
	}
	
	// Se deben habilitar los 2 Enables de los multiplexores
	E_MULTIPLEXOR_PLACA_PIN = 0;
	E_MULTIPLEXOR_PERNOS_PIN = 0;
	
	// Se deja fija la direccion de uno de los multiplexores y se varian todas las del otro
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_CONTACTOS_MAZO_66A + 1; indiceContactoPlaca++ )
	{
		// Se setean los pines del multiplexor de la placa
		switch( indiceContactoPlaca )
		{
			case 1: 	direccionContactoPlaca = 5; 	break;		// Brown
			case 2: 	direccionContactoPlaca = 3; 	break;		// Light blue
			case 3: 	direccionContactoPlaca = 2; 	break;		// Pink
			case 4: 	direccionContactoPlaca = 4; 	break;		// Orange
			case 5: 	direccionContactoPlaca = 6; 	break;		// Yellow
			case 6: 	direccionContactoPlaca = 7; 	break;		// Green
			case 7: 	direccionContactoPlaca = 1; 	break;		// Blue
			
			case 8: 	direccionContactoPlaca = 13; 	break;		// Violet
			case 9: 	direccionContactoPlaca = 14; 	break;		// Red
			case 10: 	direccionContactoPlaca = 11; 	break;		// Shield
			case 11: 	direccionContactoPlaca = 9; 	break;		// Wire
			case 12: 	direccionContactoPlaca = 15; 	break;		// Red - Peltier
			case 13: 	direccionContactoPlaca = 7; 	break;		// Green
			case 14: 	direccionContactoPlaca = 16; 	break;		// Black - Peltier
		}
		ColocarDireccionMultiplexorPlaca( direccionContactoPlaca );
		
		// Se borra el contador de contactos, mediante el cual se vera si el perno esta abierto, en corto o bien
		pernos[ indiceContactoPlaca ].totalDeContactos = 0;
		
		// Se levanta una columna de la matriz de contactos
		for( indiceContactoPernos = 1; indiceContactoPernos < TOTAL_DE_CONTACTOS_MAZO_66A + 1; indiceContactoPernos++ )
		{
			// Se deben setear los pines del multiplexor de los pernos
			switch( indiceContactoPernos )
			{
				case 1: 	direccionContactoPernos = 1; 	break;		// Brown
				case 2: 	direccionContactoPernos = 2; 	break;		// Light blue
				case 3: 	direccionContactoPernos = 3; 	break;		// Pink
				case 4: 	direccionContactoPernos = 4; 	break;		// Orange
				case 5: 	direccionContactoPernos = 5; 	break;		// Yellow
				case 6: 	direccionContactoPernos = 6; 	break;		// Green
				case 7: 	direccionContactoPernos = 7; 	break;		// Blue
				
				case 8: 	direccionContactoPernos = 8; 	break;		// Violet
				case 9: 	direccionContactoPernos = 9; 	break;		// Red
				case 10: 	direccionContactoPernos = 10; 	break;		// Shield
				case 11: 	direccionContactoPernos = 14; 	break;		// Wire
				case 12: 	direccionContactoPernos = 13; 	break;		// Red - Peltier
				case 13: 	direccionContactoPernos = 12; 	break;		// Green
				case 14: 	direccionContactoPernos = 11; 	break;		// Black - Peltier
			}
			ColocarDireccionMultiplexorContactos( direccionContactoPernos );

			// Se genera una pequenia demora para asegurarse que los multiplexores propaguen correctamente las seniales
			DEMORA_10US;
			
			// Si hay continuidad, la salida del multiplexor PERNOS_SALIDA_CONTINUIDAD tiene que tener un valor de tension distinto de cero.
			errorGeneral = TomarMuestraSimpleADC( ADC_CANAL_PERNOS ); 
			if( cuentasADC > MINIMAS_CUENTRAS_PARA_CONTINUIDAD )									// Se detecto continuidad
			{
				pernos[ indiceContactoPlaca ].contactos[ indiceContactoPernos ] = HAY_CONTINUIDAD;
				pernos[ indiceContactoPlaca ].totalDeContactos++;
				// Ademas, se registra en que ubicacion se encuentra
				pernosUbicacion[ indiceContactoPlaca ] = indiceContactoPernos;
			}
			else																					// No se detecto continuidad
				pernos[ indiceContactoPlaca ].contactos[ indiceContactoPernos ] = NO_HAY_CONTINUIDAD;
		}
	}
	
	// Se deben deshabilitar los 2 Enables de los multiplexores
	E_MULTIPLEXOR_PLACA_PIN = 1;
	E_MULTIPLEXOR_PERNOS_PIN = 1;

	
	// Se revisa el estado de los contactos, a excepcion del 5, 6 y 13 que son especiales
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_CONTACTOS_MAZO_66A + 1; indiceContactoPlaca++ )
	{
		// Se evitan el 5 y el 6
		if( indiceContactoPlaca == 5 || indiceContactoPlaca == 6 )		{	indiceContactoPlaca = 7;	}
		
		// Se evita el contacto 13
		if( indiceContactoPlaca == 13 )			{	indiceContactoPlaca++;	}
		
		// Primero se revisa si el contacto esta abierto
		if( pernos[ indiceContactoPlaca ].totalDeContactos == 0 )
		{
			condicionDelPerno[ indiceContactoPlaca ] = PERNO_ABIERTO;
			hayPernoAbierto = TRUE;
		}
		else
		{
			// Luego se revisa si esta en cortocircuito
			if( pernos[ indiceContactoPlaca ].totalDeContactos > 1 )
			{
				condicionDelPerno[ indiceContactoPlaca ] = PERNO_CORTO_CIRCUITO;
				hayPernoEnCorto = TRUE;
			}
			else
			{
				// De haber un solo contacto, se verifica que sea en la posicion correcta
				if( pernosUbicacion[ indiceContactoPlaca ] == indiceContactoPlaca )
					condicionDelPerno[ indiceContactoPlaca ] = PERNO_SIN_ERRORES;
				else
				{
					condicionDelPerno[ indiceContactoPlaca ] = PERNO_SIN_ALTERNANCIA;
					hayPernoAlternado = TRUE;
				}
			}
		}
	}
	
	
	// Perno numero 5
	if( pernos[ 5 ].totalDeContactos == 0 )
	{
		condicionDelPerno[ 5 ] = PERNO_ABIERTO;
		hayPernoAbierto = TRUE;
	}
	else
	{
		if( hayPernoAlternado == TRUE )
		{
			if( pernos[ 5 ].contactos[ 5 ] == HAY_CONTINUIDAD )
			{
				if( pernos[ 5 ].contactos[ 6 ] == HAY_CONTINUIDAD )
				{
					pernosUbicacion[ 5 ] = 6;
					condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;
					hayPernoAlternado = TRUE;
				}
				else
					condicionDelPerno[ 5 ] = PERNO_SIN_ERRORES;
			}
			else
			{
				condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;
				hayPernoAlternado = TRUE;
			}
		}
		else
		{
			// Si no estan alternados los demas contactos, no deberia registrar mas de un contacto con continuidad
			if( pernos[ 5 ].totalDeContactos > 1 )
			{
				condicionDelPerno[ 5 ] = PERNO_CORTO_CIRCUITO;
				hayPernoEnCorto = TRUE;
			}
			else
			{
				// De haber un solo contacto, se verifica que sea en la posicion correcta
				if( pernosUbicacion[ 5 ] == 5 )
					condicionDelPerno[ 5 ] = PERNO_SIN_ERRORES;
				else
				{
					condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;
					hayPernoAlternado = TRUE;
				}
			}
		}
	}
	

	// Perno numero 6
	if( pernos[ 6 ].totalDeContactos == 0 )
	{
		condicionDelPerno[ 6 ] = PERNO_ABIERTO;
		hayPernoAbierto = TRUE;
	}
	else
	{
		if( pernos[ 6 ].contactos[ 6 ] == HAY_CONTINUIDAD )
			if( pernos[ 6 ].contactos[ 5 ] == HAY_CONTINUIDAD )
			{
				pernosUbicacion[ 6 ] = 5;
				condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;
				hayPernoAlternado = TRUE;
			}
			else
				condicionDelPerno[ 6 ] = PERNO_SIN_ERRORES;
		else
		{
			condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;
			hayPernoAlternado = TRUE;
		}
	}
	
	// Perno numero 13
	if( pernos[ 13 ].totalDeContactos == 0 )
	{
		condicionDelPerno[ 13 ] = PERNO_ABIERTO;
		hayPernoAbierto = TRUE;
	}
	else
	{
		if( pernos[ 13 ].contactos[ 13 ] == HAY_CONTINUIDAD )
			condicionDelPerno[ 13 ] = PERNO_SIN_ERRORES;
		else
		{
			condicionDelPerno[ 13 ] = PERNO_SIN_ALTERNANCIA;
			hayPernoAlternado = TRUE;
		}
	}
	
	if( hayPernoAbierto == TRUE )
		return( PERNO_ABIERTO );
	
	if( hayPernoEnCorto == TRUE )
		return( PERNO_CORTO_CIRCUITO );
	
	if( hayPernoAlternado == TRUE )
		return( PERNO_SIN_ALTERNANCIA );
		
	// Si llega hasta aca, es porque todos los contactos estan correctos
	return( PERNO_SIN_ERRORES );
*/}



/****************************************************************/
/* InformarCablesAbiertos										*/
/*  															*/
/*  Muestra en pantalla la indicacion de que cables se			*/
/*  encuentran abiertos											*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InformarCablesAbiertos ( void )
{
	// Se informa que hay un error por que algunos pernos estan abiertos
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "Abiert:    xx-xx-xx " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       0, 20, "           xx-xx-xx " );
	
	/* *** RENGLON SUPERIOR: CABLES DEL 1 AL 3 *** */
	if( condicionDelPerno[ 1 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 11, 2, "01" );				// Brown
	if( condicionDelPerno[ 2 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 14, 2, "02" );				// Light blue
	if( condicionDelPerno[ 3 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 17, 2, "03" );				// Pink

	/* *** RENGLON MEDIO SUPERIOR: CABLES DEL 4 AL 7 *** */
	if( condicionDelPerno[ 4 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 9, 2, "04" );		// Orange
	if( condicionDelPerno[ 5 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 12, 2, "05" );		// Yellow
	if( condicionDelPerno[ 6 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 15, 2, "06" );		// Green
	if( condicionDelPerno[ 7 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 18, 2, "07" );		// Blue
	
	/* *** RENGLON MEDIO INFERIOR: CABLES DEL 8 AL 11 *** */
	if( condicionDelPerno[ 8 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 9, 2, "08" );		// Violet
	if( condicionDelPerno[ 9 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 12, 2, "09" );		// Red
	if( condicionDelPerno[ 10 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 15, 2, "10" );		// Shield
	if( condicionDelPerno[ 11 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 18, 2, "11" );		// Wire
	
	
	/* *** RENGLON INFERIOR: CABLES DEL 12 AL 14 *** */
	if( condicionDelPerno[ 12 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 11, 2, "12" );				// Red - Peltier
	if( condicionDelPerno[ 13 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 14, 2, "13" );				// Green
	if( condicionDelPerno[ 14 ] == PERNO_ABIERTO )
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 17, 2, "14" );				// Black - Peltier
}



/****************************************************************/
/* InformarCablesEnCorto										*/
/*  															*/
/*  Muestra en pantalla la indicacion de que cables se			*/
/*  encuentran en corto circuito entre si						*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InformarCablesEnCorto ( void )
{
	// Se informa que hay un error por que algunos pernos estan abiertos
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "Corto:     xx-xx-xx " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       0, 20, "           xx-xx-xx " );
	
	/* *** RENGLON SUPERIOR: CABLES DEL 1 AL 3 *** */
	if( condicionDelPerno[ 1 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 11, 2, "01" );				// Brown
	if( condicionDelPerno[ 2 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 14, 2, "02" );				// Light blue
	if( condicionDelPerno[ 3 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR, 17, 2, "03" );				// Pink

	/* *** RENGLON MEDIO SUPERIOR: CABLES DEL 4 AL 7 *** */
	if( condicionDelPerno[ 4 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 9, 2, "04" );		// Orange
	if( condicionDelPerno[ 5 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 12, 2, "05" );		// Yellow
	if( condicionDelPerno[ 6 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 15, 2, "06" );		// Green
	if( condicionDelPerno[ 7 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 18, 2, "07" );		// Blue
	
	/* *** RENGLON MEDIO INFERIOR: CABLES DEL 8 AL 11 *** */
	if( condicionDelPerno[ 8 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 9, 2, "08" );		// Violet
	if( condicionDelPerno[ 9 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 12, 2, "09" );		// Red
	if( condicionDelPerno[ 10 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 15, 2, "10" );		// Shield
	if( condicionDelPerno[ 11 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 18, 2, "11" );		// Wire
	
	
	/* *** RENGLON INFERIOR: CABLES DEL 12 AL 14 *** */
	if( condicionDelPerno[ 12 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 11, 2, "12" );				// Red - Peltier
	if( condicionDelPerno[ 13 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 14, 2, "13" );				// Green
	if( condicionDelPerno[ 14 ] == PERNO_CORTO_CIRCUITO )
		errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR, 17, 2, "14" );				// Black - Peltier

}



/****************************************************************/
/* InformarCablesConMalUbicacion								*/
/*  															*/
/*  Muestra en pantalla la indicacion de que cables se			*/
/*  encuentran en una posicion incorrecta						*/
/*  															*/
/*  Recibe: Nada												*/
/*  Devuelve: Nada												*/
/****************************************************************/
void InformarCablesConMalUbicacion ( void )
{
	// Variable auxiliar
	char	indiceContactoPlaca;
	char	renglon;
	char	offset;
	
	// Se informa que hay un error por que algunos pernos estan abiertos
	BorrarLCD();
	errorGeneral = EscribirMensajeLCD( RENGLON_SUPERIOR,       0, 20, "Alter:     xx-xx-xx " );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_SUPERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_MEDIO_INFERIOR, 0, 20, "         xx-xx-xx-xx" );
	errorGeneral = EscribirMensajeLCD( RENGLON_INFERIOR,       0, 20, "           xx-xx-xx " );
	
	for( indiceContactoPlaca = 1; indiceContactoPlaca < TOTAL_DE_CONTACTOS_MAZO_66A + 1; indiceContactoPlaca++ )
	{
		// Si el perno esta en su lugar correcto, no se indica nada
		if( condicionDelPerno[ indiceContactoPlaca ] == PERNO_SIN_ALTERNANCIA )
		{
			// Se calcula el offset y el renglon para la ubicacion de cada indicacion
			switch( indiceContactoPlaca )
			{
				case 1:
					renglon = RENGLON_SUPERIOR;
					offset = 11;
					break;
				case 2:
					renglon = RENGLON_SUPERIOR;
					offset = 14;
					break;
				case 3:
					renglon = RENGLON_SUPERIOR;
					offset = 17;
					break;
					
				case 4:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 9;
					break;
				case 5:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 12;
					break;
				case 6:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 15;
					break;
				case 7:
					renglon = RENGLON_MEDIO_SUPERIOR;
					offset = 18;
					break;
					
				case 8:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 9;
					break;
				case 9:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 12;
					break;
				case 10:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 15;
					break;
				case 11:
					renglon = RENGLON_MEDIO_INFERIOR;
					offset = 18;
					break;
					
				case 12:
					renglon = RENGLON_INFERIOR;
					offset = 11;
					break;
				case 13:
					renglon = RENGLON_INFERIOR;
					offset = 14;
					break;
				case 14:
					renglon = RENGLON_INFERIOR;
					offset = 17;
					break;
			}
			
			// Se imprime la ubucacion que se encuentra en un lugar incorrecto
			EscribirUbicacion( pernosUbicacion[ indiceContactoPlaca ], renglon, offset );
		}
	}
}






















/****************************************************************************************************************/
/*  							FIRMWARE PARA CONTOLAR LOS MULTIPLEXORES DE LOS PERNOS							*/
/****************************************************************************************************************/


/****************************************************************/
/* ObtenerMatrizDePernos										*/
/*  															*/
/*  Levanta la matriz de contactos de los pernos. Solo se marca	*/
/*  si hay continuidad entre las direcciones utilizadas y la	*/
/*  cantidad de contactos para cada direccion.					*/
/*  															*/
/*  Recibe: El target, para saber la cantidad de pernos			*/
/*  Devuelve: Nada												*/
/****************************************************************/
void ObtenerMatrizDePernos ( char target )
{
	// Variables auxiliares
	char indiceContactoPernos;
	char indiceContactoPlaca;
	char totalDeContactos;
	
	// Se selecciona el total de contactos en funcion del target
	switch( target )
	{
		// Lugar 1
		case CABLE_MAZO_001:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_001 + 1;
			break;
		case CABLE_MAZO_041:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_041 + 1;
			break;
		case CABLE_MAZO_105:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_105 + 1;
			break;
			
		// Lugar 2
		case CABLE_MAZO_002:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_002 + 1;
			break;
		case CABLE_MAZO_042:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_042 + 1;
			break;
		case CABLE_MAZO_072:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_072 + 1;
			break;
		case CABLE_MAZO_106:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_106 + 1;
			break;
			
		// Lugar 3
		case CABLE_MAZO_003:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_003 + 1;
			break;
		case CABLE_MAZO_004:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_004 + 1;
			break;
		case CABLE_MAZO_005:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_005 + 1;
			break;
		case CABLE_MAZO_026:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_026 + 1;
			break;
		case CABLE_MAZO_043:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_043 + 1;
			break;
		case CABLE_MAZO_044:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_044 + 1;
			break;
		case CABLE_MAZO_060:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_060 + 1;
			break;
		case CABLE_MAZO_073:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_073 + 1;
			break;
		case CABLE_MAZO_074:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_074 + 1;
			break;
		case CABLE_MAZO_110:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_110 + 1;
			break;
			
		// Lugar 4
		case CABLE_MAZO_006:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_006 + 1;
			break;
		case CABLE_MAZO_007:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_007 + 1;
			break;
		case CABLE_MAZO_045:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_045 + 1;
			break;
		case CABLE_MAZO_046:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_046 + 1;
			break;
		case CABLE_MAZO_075:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_075 + 1;
			break;
		case CABLE_MAZO_107:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_107 + 1;
			break;
			
		// Lugar 5
		case CABLE_MAZO_012:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_012 + 1;
			break;
		case CABLE_MAZO_051:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_051 + 1;
			break;
			
		// Lugar 6
		case CABLE_MAZO_025:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_025 + 1;
			break;
		case CABLE_MAZO_071:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_071 + 1;
			break;
		case CABLE_MAZO_083:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_083 + 1;
			break;
			
		// Lugar 7
		case CABLE_MAZO_047:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_047 + 1;
			break;
		case CABLE_MAZO_076:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_076 + 1;
			break;
		
		// Lugar 8
		case CABLE_MAZO_011:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_011 + 1;
			break;
		case CABLE_MAZO_050:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_050 + 1;
			break;
		case CABLE_MAZO_108:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_108 + 1;
			break;
			
		// Lugar 9
		case CABLE_MAZO_023:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_023 + 1;
			break;
		case CABLE_MAZO_081:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_081 + 1;
			break;
			
		// Lugar 10
		case CABLE_MAZO_021:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_021 + 1;
			break;
		case CABLE_MAZO_079:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_079 + 1;
			break;
			
		// Lugar 11
		case CABLE_MAZO_031:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_031 + 1;
			break;

		// Lugar 12
		case CABLE_MAZO_038:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_038 + 1;
			break;
			
		// Lugar 13
		case CABLE_MAZO_078:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_078 + 1;
			break;
			
		// Lugar 14
		case CABLE_MAZO_030:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_030 + 1;
			break;
			
		// Lugar 15
		case CABLE_MAZO_090:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_090 + 1;
			break;
			
		// Lugar 16
		case CABLE_MAZO_009:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_009 + 1;
			break;
		case CABLE_MAZO_048:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_048 + 1;
			break;
		case CABLE_MAZO_077:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_077 + 1;
			break;
			
		// Lugar 17
		case CABLE_MAZO_022:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_022 + 1;
			break;
			
		// Lugar 18
		case CABLE_MAZO_024:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_024 + 1;
			break;
		case CABLE_MAZO_109:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_109 + 1;
			break;
			
		// Lugar 19
		case CABLE_MAZO_082:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_082 + 1;
			break;
			
		// Lugar 20
		case CABLE_MAZO_096:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_096 + 1;
			break;
			
		// Lugar 21
		case CABLE_MAZO_097_A:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_097_A + 1;
			break;
		case CABLE_MAZO_097_B:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_097_B + 1;
			break;
			
		// Lugar 23
		case CABLE_MAZO_032:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_032 + 1;
			break;
		case CABLE_MAZO_066:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_066 + 1;
			break;
			
		// Lugar 24
		case CABLE_MAZO_111:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_111 + 1;
			break;
			
		// Lugar 25
		case CABLE_MAZO_035:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_035 + 1;
			break;
			
		// Lugar 26
		case CABLE_MAZO_034:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_034 + 1;
			break;
			
		// Lugar del cable de telefono
		case CABLE_MAZO_000:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_000 + 1;
			break;

	}
	
	// Se borran los contadores
	hayPernoAbierto = FALSE;
	hayPernoEnCorto = FALSE;
	hayPernoEnCortoCon5V = FALSE;
	hayPernoEnCortoElMazo = FALSE;
	hayPernoAlternado = FALSE;
	
	// Se borran los indicadores
	for( indiceContactoPlaca = 1; indiceContactoPlaca <= TOTAL_DE_CONTACTOS_MULTIPLEXORES; indiceContactoPlaca++ )
	{
		condicionDelPerno[ indiceContactoPlaca ] = PERNO_SIN_ERRORES;
		pernosUbicacion[ indiceContactoPlaca ] = 0;
		pernos[ indiceContactoPlaca ].totalDeContactos = 0;
		for( indiceContactoPernos = 1; indiceContactoPernos < totalDeContactos; indiceContactoPernos++ )
			pernos[ indiceContactoPlaca ].contactos[ indiceContactoPernos ] = NO_HAY_CONTINUIDAD;
	}
	
	
	// Se deben habilitar los 2 Enables de los multiplexores
	E_MULTIPLEXOR_PLACA_PIN = 0;
	E_MULTIPLEXOR_PERNOS_PIN = 0;
	
	// Se deja fija la direccion de uno de los multiplexores y se varian todas las del otro
	for( indiceContactoPlaca = 1; indiceContactoPlaca < totalDeContactos; indiceContactoPlaca++ )
	{
		// Se toman como fijas las posiciones del multiplexor de la placa, ya que el conector que va aca esta correctamente armado
		ColocarDireccionMultiplexorPlaca( indiceContactoPlaca, target );
		
		// Se levanta una columna de la matriz de contactos
		for( indiceContactoPernos = 1; indiceContactoPernos < totalDeContactos; indiceContactoPernos++ )
		{
			// Los contactos desde el lado de los pernos pueden estar en ubicaciones incorrectas
			ColocarDireccionMultiplexorContactos( indiceContactoPernos, target );
			
			// Si hay continuidad, la salida del multiplexor PERNOS_SALIDA_CONTINUIDAD tiene que tener un valor de tension distinto de cero.
			errorGeneral = TomarMuestraSimpleADC( ADC_CANAL_PERNOS ); 
			
			if( cuentasADC > MINIMAS_CUENTRAS_PARA_CONTINUIDAD )									// Se detecto continuidad
			{
				pernos[ indiceContactoPlaca ].contactos[ indiceContactoPernos ] = HAY_CONTINUIDAD;
				pernos[ indiceContactoPlaca ].totalDeContactos++;
				// Ademas, se registra en que ubicacion se encuentra
				pernosUbicacion[ indiceContactoPlaca ] = indiceContactoPernos;
			}
			else																					// No se detecto continuidad
				pernos[ indiceContactoPlaca ].contactos[ indiceContactoPernos ] = NO_HAY_CONTINUIDAD;
		}
	}
	
	// Se deben deshabilitar los 2 Enables de los multiplexores
	E_MULTIPLEXOR_PLACA_PIN = 1;
	E_MULTIPLEXOR_PERNOS_PIN = 1;
	
	return;

}



/****************************************************************/
/* ColocarDireccionMultiplexorContactos							*/
/*  															*/
/*  Se encarga de modificar los pines del multiplexor para los	*/
/*  contactos de los pernos, para poder seleccionar la			*/
/*  direccion suministrada.										*/
/*  															*/
/*  Recibe: La direccion a colocar								*/
/*  Devuelve: Nada												*/
/****************************************************************/
void ColocarDireccionMultiplexorContactos( char direccion, char target )
{
	// Variable auxiliar
	char auxiliar;
	
	/* *** Se traduce la direccion logica solicitada a la direccion fisica necesaria en los pines segun el target *** */
	switch( target )
	{
	
		// Lugar 1
		case CABLE_MAZO_001:
			switch( direccion )
			{
				case 1:		direccion = 4;	break;
				case 2:		direccion = 3;	break;
				case 3:		direccion = 1;	break;
				case 4:		direccion = 0;	break;
			}
			break;
		case CABLE_MAZO_041:
			switch( direccion )
			{
				case 1:		direccion = 4;	break;
				case 2:		direccion = 3;	break;
				case 3:		direccion = 1;	break;
				case 4:		direccion = 0;	break;
			}
			break;
		case CABLE_MAZO_105:
			switch( direccion )
			{
				case 1:		direccion = 4;	break;
				case 2:		direccion = 3;	break;
				case 3:		direccion = 1;	break;
				case 4:		direccion = 0;	break;
			}
			break;
		
		// Lugar 2
		case CABLE_MAZO_002:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_042:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_072:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_106:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
			
		// Lugar 3
		case CABLE_MAZO_003:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_004:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_005:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 3;	break;
			}
			break;
		case CABLE_MAZO_026:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_043:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_044:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_060:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_073:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
		case CABLE_MAZO_074:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 3;	break;
			}
			break;
		case CABLE_MAZO_110:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
			}
			break;
			
		// Lugar 4
		case CABLE_MAZO_006:
			switch( direccion )
			{
				case 1:		direccion = 3;	break;
				case 2:		direccion = 2;	break;
				case 3:		direccion = 1;	break;
			}
			break;
		case CABLE_MAZO_007:
			switch( direccion )
			{
				case 1:		direccion = 3;	break;
				case 2:		direccion = 2;	break;
				case 3:		direccion = 1;	break;
				case 4:		direccion = 0;	break;
			}
			break;
		case CABLE_MAZO_045:
			switch( direccion )
			{
				case 1:		direccion = 3;	break;
				case 2:		direccion = 2;	break;
				case 3:		direccion = 1;	break;
			}
			break;
		case CABLE_MAZO_046:
			switch( direccion )
			{
				case 1:		direccion = 3;	break;
				case 2:		direccion = 2;	break;
				case 3:		direccion = 1;	break;
				case 4:		direccion = 0;	break;
			}
			break;
		case CABLE_MAZO_075:
			switch( direccion )
			{
				case 1:		direccion = 3;	break;
				case 2:		direccion = 2;	break;
				case 3:		direccion = 1;	break;
			}
			break;
		case CABLE_MAZO_107:
			switch( direccion )
			{
				case 1:		direccion = 3;	break;
				case 2:		direccion = 2;	break;
				case 3:		direccion = 1;	break;
				case 4:		direccion = 0;	break;
			}
			break;
			
			
		// Lugar 5
		case CABLE_MAZO_012:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 3;	break;
			}
			break;
		case CABLE_MAZO_051:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 3;	break;
			}
			break;
			
			
		// Lugar 6
		case CABLE_MAZO_025:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
			}
			break;
		case CABLE_MAZO_071:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
			}
			break;
		case CABLE_MAZO_083:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
			}
			break;
			
			
		// Lugar 7
		case CABLE_MAZO_047:
			switch( direccion )
			{
				case 1:		direccion = 4;	break;
				case 2:		direccion = 3;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 1;	break;
				case 5:		direccion = 0;	break;
			}
			break;
		case CABLE_MAZO_076:
			switch( direccion )
			{
				case 1:		direccion = 4;	break;
				case 2:		direccion = 3;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 1;	break;
				case 5:		direccion = 0;	break;
			}
			break;
		
			
		// Lugar 8
		case CABLE_MAZO_011:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 3;	break;
			}
			break;
		case CABLE_MAZO_050:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 3;	break;
			}
			break;
		case CABLE_MAZO_108:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 3;	break;
			}
			break;
			

		// Lugar 9
		case CABLE_MAZO_023:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
			}
			break;
		case CABLE_MAZO_081:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
			}
			break;
			
			
		// Lugar 10
		case CABLE_MAZO_021:
			switch( direccion )
			{
				case 1:		direccion = 2;	break;
				case 2:		direccion = 0;	break;
			}
			break;
		case CABLE_MAZO_079:
			switch( direccion )
			{
				case 1:		direccion = 2;	break;
				case 2:		direccion = 0;	break;
			}
			break;
			
			
		// Lugar 11
		case CABLE_MAZO_031:
			switch( direccion )
			{
				case 1:		direccion = 1;	break;
				case 2:		direccion = 0;	break;
			}
			break;
			
			
		// Lugar 12
		case CABLE_MAZO_038:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 2;	break;
				case 3:		direccion = 1;	break;
				case 4:		direccion = 3;	break;
			}
			break;
			
			
		// Lugar 13
		case CABLE_MAZO_078:
			switch( direccion )
			{
				case 1:		direccion = 1;	break;
				case 2:		direccion = 0;	break;
			}
			break;
			
			
		// Lugar 14
		case CABLE_MAZO_030:
			switch( direccion )
			{
				case 1:		direccion = 2;	break;
				case 2:		direccion = 3;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 0;	break;
				case 5:		direccion = 1;	break;
				case 6:		direccion = 0;	break;
			}
			break;
			
			
		// Lugar 15
		case CABLE_MAZO_090:
			switch( direccion )
			{
				case 1:		direccion = 1;	break;
				case 2:		direccion = 0;	break;
			}
			break;
			
			
		// Lugar 16
		case CABLE_MAZO_009:
			switch( direccion )
			{
				case 1:			direccion = 8;	break;
				case 2:			direccion = 9;	break;
				case 3:			direccion = 10;	break;
				case 4:			direccion = 11;	break;
				case 5:			direccion = 12;	break;
				case 6:			direccion = 13;	break;
				case 7:			direccion = 14;	break;
				case 8:			direccion = 15;	break;
			}
			break;
			
		case CABLE_MAZO_048:
			switch( direccion )
			{
				case 1:			direccion = 8;	break;
				case 2:			direccion = 9;	break;
				case 3:			direccion = 10;	break;
				case 4:			direccion = 11;	break;
				case 5:			direccion = 12;	break;
				case 6:			direccion = 13;	break;
				case 7:			direccion = 14;	break;
				case 8:			direccion = 15;	break;
			}
			break;
			
		case CABLE_MAZO_077:
			switch( direccion )
			{
				case 1:			direccion = 8;	break;
				case 2:			direccion = 9;	break;
				case 3:			direccion = 10;	break;
				case 4:			direccion = 11;	break;
				case 5:			direccion = 12;	break;
				case 6:			direccion = 13;	break;
				case 7:			direccion = 14;	break;
				case 8:			direccion = 15;	break;
			}
			break;
			
			
		// Lugar 17
		case CABLE_MAZO_022:
			switch( direccion )
			{
				case 1:		direccion = 1;	break;
				case 2:		direccion = 0;	break;
			}
			break;
			
			
		// Lugar 18
		case CABLE_MAZO_024:
			switch( direccion )
			{				
				case 1:			direccion = 1;	break;
				case 2:			direccion = 3;	break;
				case 3:			direccion = 9;	break;
				case 4:			direccion = 2;	break;
				case 5:			direccion = 10;	break;
				case 6:			direccion = 11;	break;
				case 7:			direccion = 0;	break;
				case 8:			direccion = 4;	break;
				case 9:			direccion = 8;	break;
			}
			break;
		case CABLE_MAZO_109:
			switch( direccion )
			{
				case 1:			direccion = 1;	break;
				case 2:			direccion = 9;	break;
				case 3:			direccion = 0;	break;
				case 4:			direccion = 8;	break;
				case 5:			direccion = 2;	break;
				case 6:			direccion = 10;	break;
				case 7:			direccion = 11;	break;

			}
			break;
			
			
		// Lugar 19
		case CABLE_MAZO_082:
			switch( direccion )
			{				
				case 3:			direccion = 3;	break;
				case 2:			direccion = 6;	break;
				case 1:			direccion = 4;	break;
				case 4:			direccion = 8;	break;
				case 5:			direccion = 9;	break;
				case 6:			direccion = 10;	break;
				case 7:			direccion = 5;	break;
			}
			break;
			
			
		// Lugar 20
		case CABLE_MAZO_096:
			switch( direccion )
			{				
				case 1:			direccion = 10;	break;
				case 2:			direccion = 14;	break;
				case 3:			direccion = 12;	break;
				case 4:			direccion = 13;	break;
				case 5:			direccion = 11;	break;
				case 6:			direccion = 9;	break;
				case 7:			direccion = 8;	break;
			}
			break;
			
			
		// Lugar 21
		case CABLE_MAZO_097_A:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;		// Brown
				case 2: 	direccion = 2; 		break;		// Light blue
				case 3: 	direccion = 1; 		break;		// Pink
				case 4: 	direccion = 3; 		break;		// Orange
				case 5: 	direccion = 4; 		break;		// Yellow
				case 6: 	direccion = 5; 		break;		// Green
				case 7: 	direccion = 6; 		break;		// Blue
				
				case 8: 	direccion = 7; 		break;		// Violet
				case 9: 	direccion = 9; 		break;		// Red
				case 10: 	direccion = 15;		break;		// Shield
				case 11: 	direccion = 8; 		break;		// Wire
				case 12: 	direccion = 12; 	break;		// Red - Peltier
				case 13: 	direccion = 11; 	break;		// Green
				case 14: 	direccion = 10; 	break;		// Black - Peltier
				
				case 15:	direccion = 13; 	break;		// Black - Peltier
				case 16:	direccion = 14; 	break;		// Black - Peltier
			}
			break;
			
		case CABLE_MAZO_097_B:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;		// Brown
				case 2: 	direccion = 1; 		break;		// Light blue
				case 3: 	direccion = 2; 		break;		// Pink
				case 4: 	direccion = 3; 		break;		// Orange
				case 5: 	direccion = 4; 		break;		// Yellow
				case 6: 	direccion = 5; 		break;		// Green
				case 7: 	direccion = 6; 		break;		// Blue
			}
			break;
	
			
		// Lugar 23
			case CABLE_MAZO_032:
				switch( direccion )
				{
					case 1: 	direccion = 0; 		break;		// Brown
					case 2: 	direccion = 1; 		break;		// Light blue
					case 3: 	direccion = 2; 		break;		// Pink
					case 4: 	direccion = 3; 		break;		// Orange
					case 5: 	direccion = 4; 		break;		// Yellow
					case 6: 	direccion = 5; 		break;		// Green
					case 7: 	direccion = 6; 		break;		// Blue
					
					case 8: 	direccion = 7; 		break;		// Violet
					case 9: 	direccion = 15; 	break;		// Red
					case 10: 	direccion = 8;		break;		// Shield
					case 11: 	direccion = 9; 		break;		// Wire
					case 12: 	direccion = 12; 	break;		// Red - Peltier
					case 13: 	direccion = 5; 		break;		// Green
					case 14: 	direccion = 10; 	break;		// Black - Peltier
				}
				break;
				
		case CABLE_MAZO_066:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;		// Brown
				case 2: 	direccion = 1; 		break;		// Light blue
				case 3: 	direccion = 2; 		break;		// Pink
				case 4: 	direccion = 3; 		break;		// Orange
				case 5: 	direccion = 4; 		break;		// Yellow
				case 6: 	direccion = 5; 		break;		// Green
				case 7: 	direccion = 6; 		break;		// Blue
				
				case 8: 	direccion = 7; 		break;		// Violet
				case 9: 	direccion = 15; 	break;		// Red
				case 10: 	direccion = 8;		break;		// Shield
				case 11: 	direccion = 9; 		break;		// Wire
				case 12: 	direccion = 12; 	break;		// Red - Peltier
				case 13: 	direccion = 5; 		break;		// Green
				case 14: 	direccion = 10; 	break;		// Black - Peltier
			}
			break;
			
			
		// Lugar 24
		case CABLE_MAZO_111:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;		// Brown
				case 2: 	direccion = 1; 		break;		// Light blue
				case 3: 	direccion = 2; 		break;		// Pink
				case 4: 	direccion = 3; 		break;		// Orange
				case 5: 	direccion = 4; 		break;		// Yellow
				case 6: 	direccion = 5; 		break;		// Green
				case 7: 	direccion = 6; 		break;		// Blue
			}
			break;
			
			
		// Lugar 25
		case CABLE_MAZO_035:
			switch( direccion )
			{
				case 1: 	direccion = 14; 	break;		// Light blue
				case 2: 	direccion = 12; 	break;		// Pink
				case 3: 	direccion = 11; 	break;		// Yellow
				case 4: 	direccion = 9; 		break;		// Green
			}
			break;
			
			
		// Lugar 26
		case CABLE_MAZO_034:
			switch( direccion )
			{
				case 1: 	direccion = 1; 		break;		// Red
				case 2: 	direccion = 3; 		break;		// Green
				case 3: 	direccion = 7; 		break;		// Negro
				case 4: 	direccion = 7; 		break;		// Marron
				case 5: 	direccion = 7; 		break;		// Blanco
				case 6:		direccion = 4; 		break;		// Malla del marron
				case 7:		direccion = 6; 		break;		// Malla del negro y del blanco
			}
			break;
			
			
		// Lugar para el cable de telefono
		case CABLE_MAZO_000:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 3;	break;
			}
			break;
			
	}
	
	// Se modifican los pines para colocar la direccion solicitada
	auxiliar = direccion;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S0 de los pernos
		S0_MULTIPLEXOR_PERNOS_PIN = 1;
	}
	else
	{
		// Se borra el pin S0 de los pernos
		S0_MULTIPLEXOR_PERNOS_PIN = 0;
	}
	
	auxiliar = direccion >> 1;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S1 de los pernos
		S1_MULTIPLEXOR_PERNOS_PIN = 1;
	}
	else
	{
		// Se borra el pin S1 de los pernos
		S1_MULTIPLEXOR_PERNOS_PIN = 0;
	}
	
	auxiliar = direccion >> 2;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S2 de los pernos
		S2_MULTIPLEXOR_PERNOS_PIN = 1;
	}
	else
	{
		// Se borra el pin S2 de los pernos
		S2_MULTIPLEXOR_PERNOS_PIN = 0;
	}
	
	auxiliar = direccion >> 3;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S3 de los pernos
		S3_MULTIPLEXOR_PERNOS_PIN = 1;
	}
	else
	{
		// Se borra el pin S3 de los pernos
		S3_MULTIPLEXOR_PERNOS_PIN = 0;
	}
}



/****************************************************************/
/* ColocarDireccionMultiplexorPlaca								*/
/*  															*/
/*  Se encarga de modificar los pines del multiplexor para los	*/
/*  contactos de la placa, para poder seleccionar la direccion	*/
/*  suministrada.												*/
/*  															*/
/*  Recibe: La direccion a colocar								*/
/*  Devuelve: Nada												*/
/****************************************************************/
void ColocarDireccionMultiplexorPlaca( char direccion, char target )
{
	// Variable auxiliar
	char auxiliar;
	
	/* *** Se traduce la direccion logica solicitada a la direccion fisica necesaria en los pines segun el target *** */
	switch( target )
	{
	
		// Lugar 1
		case CABLE_MAZO_001:
			switch( direccion )
			{
				case 1:			direccion = 12;	break;
				case 2:			direccion = 11;	break;
				case 3:			direccion = 14;	break;
				case 4:			direccion = 13;	break;
			}
			break;
		case CABLE_MAZO_041:
			switch( direccion )
			{
				case 1:			direccion = 12;	break;
				case 2:			direccion = 11;	break;
				case 3:			direccion = 14;	break;
				case 4:			direccion = 13;	break;
			}
			break;
		case CABLE_MAZO_105:
			switch( direccion )
			{
				case 1:			direccion = 12;	break;
				case 2:			direccion = 11;	break;
				case 3:			direccion = 14;	break;
				case 4:			direccion = 13;	break;
			}
			break;
		
		// Lugar 2
		case CABLE_MAZO_002:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_042:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_072:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_106:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
			
		// Lugar 3
		case CABLE_MAZO_003:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_004:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_005:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
				case 4:		direccion = 12;	break;
			}
			break;
		case CABLE_MAZO_026:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_043:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_044:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_060:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_073:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_074:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
				case 4:		direccion = 12;	break;
			}
			break;
		case CABLE_MAZO_110:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
			
			
		// Lugar 4
		case CABLE_MAZO_006:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 14;	break;
			}
			break;
		case CABLE_MAZO_007:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 14;	break;
				case 4:		direccion = 13;	break;
			}
			break;
		case CABLE_MAZO_045:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 14;	break;
				case 3:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_046:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 14;	break;
				case 4:		direccion = 13;	break;
			}
			break;
		case CABLE_MAZO_075:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 14;	break;
			}
			break;
		case CABLE_MAZO_107:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 14;	break;
				case 4:		direccion = 13;	break;
			}
			break;
			
			
		// Lugar 5
		case CABLE_MAZO_012:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 14;	break;
				case 4:		direccion = 13;	break;
			}
			break;
		case CABLE_MAZO_051:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 14;	break;
				case 4:		direccion = 13;	break;
			}
			break;
			
			
		// Lugar 6
		case CABLE_MAZO_025:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_071:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_083:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
			}
			break;
			
			
		// Lugar 7
		case CABLE_MAZO_047:
			switch( direccion )
			{
				case 1:		direccion = 15;	break;
				case 2:		direccion = 13;	break;
				case 3:		direccion = 14;	break;
				case 4:		direccion = 12;	break;
				case 5:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_076:
			switch( direccion )
			{
				case 1:		direccion = 15;	break;
				case 2:		direccion = 13;	break;
				case 3:		direccion = 14;	break;
				case 4:		direccion = 12;	break;
				case 5:		direccion = 11;	break;
			}
			break;
			
			
		// Lugar 8
		case CABLE_MAZO_011:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 13;	break;
				case 4:		direccion = 14;	break;
			}
			break;
		case CABLE_MAZO_050:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 13;	break;
				case 4:		direccion = 14;	break;
			}
			break;
		case CABLE_MAZO_108:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
				case 3:		direccion = 13;	break;
				case 4:		direccion = 14;	break;
			}
			break;
			
			
		// Lugar 9
		case CABLE_MAZO_023:
			switch( direccion )
			{
				case 1:		direccion = 11;	break;
				case 2:		direccion = 12;	break;
			}
			break;
		case CABLE_MAZO_081:
			switch( direccion )
			{
				case 1:		direccion = 11;	break;
				case 2:		direccion = 12;	break;
			}
			break;
			
			
		// Lugar 10
		case CABLE_MAZO_021:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
			}
			break;
		case CABLE_MAZO_079:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
			}
			break;
			
			
		// Lugar 11
		case CABLE_MAZO_031:
			switch( direccion )
			{
				case 1:		direccion = 12;	break;
				case 2:		direccion = 11;	break;
			}
			break;
			
			
		// Lugar 12
		case CABLE_MAZO_038:
			switch( direccion )
			{
				case 1:		direccion = 13;	break;
				case 2:		direccion = 12;	break;
				case 3:		direccion = 11;	break;
				case 4:		direccion = 14;	break;
			}
			break;
			
			
		// Lugar 13
		case CABLE_MAZO_078:
			switch( direccion )
			{
				case 1:		direccion = 11;	break;
				case 2:		direccion = 12;	break;
			}
			break;
			
			
		// Lugar 14
		case CABLE_MAZO_030:
			switch( direccion )
			{
				case 1:		direccion = 6;	break;
				case 2:		direccion = 5;	break;
				case 3:		direccion = 4;	break;
				case 4:		direccion = 3;	break;
				case 5:		direccion = 2;	break;
				case 6:		direccion = 1;	break;
			}
			break;
			
			
		// Lugar 15
		case CABLE_MAZO_090:
			switch( direccion )
			{
				case 1:		direccion = 11;	break;
				case 2:		direccion = 12;	break;
			}
			break;
			
			
		// Lugar 16
		case CABLE_MAZO_009:
			switch( direccion )
			{
				case 1:			direccion = 1;	break;
				case 2:			direccion = 2;	break;
				case 3:			direccion = 5;	break;
				case 4:			direccion = 6;	break;
				case 5:			direccion = 3;	break;
				case 6:			direccion = 4;	break;
				case 7:			direccion = 7;	break;
				case 8:			direccion = 0;	break;
			}
			break;
			
		case CABLE_MAZO_048:
			switch( direccion )
			{
				case 1:			direccion = 1;	break;
				case 2:			direccion = 2;	break;
				case 3:			direccion = 5;	break;
				case 4:			direccion = 6;	break;
				case 5:			direccion = 3;	break;
				case 6:			direccion = 4;	break;
				case 7:			direccion = 7;	break;
				case 8:			direccion = 0;	break;
			}
			break;
				
		case CABLE_MAZO_077:
			switch( direccion )
			{
				case 1:			direccion = 1;	break;
				case 2:			direccion = 2;	break;
				case 3:			direccion = 5;	break;
				case 4:			direccion = 6;	break;
				case 5:			direccion = 3;	break;
				case 6:			direccion = 4;	break;
				case 7:			direccion = 7;	break;
				case 8:			direccion = 0;	break;
			}
			break;
			
			
		// Lugar 17
		case CABLE_MAZO_022:
			switch( direccion )
			{
				case 1:		direccion = 11;	break;
				case 2:		direccion = 12;	break;
			}
			break;
			
			
		// Lugar 18
		case CABLE_MAZO_024:
			switch( direccion )
			{
				case 1:			direccion = 8;	break;
				case 2:			direccion = 8;	break;
				case 3:			direccion = 8;	break;
				case 4:			direccion = 9;	break;
				case 5:			direccion = 9;	break;
				case 6:			direccion = 9;	break;
				case 7:			direccion = 15;	break;
				case 8:			direccion = 15;	break;
				case 9:			direccion = 15;	break;
			}
			break;
		case CABLE_MAZO_109:
			switch( direccion )
			{
				case 1:			direccion = 8;	break;
				case 2:			direccion = 8;	break;
				case 3:			direccion = 15;	break;
				case 4:			direccion = 15;	break;
				case 5:			direccion = 9;	break;
				case 6:			direccion = 9;	break;
				case 7:			direccion = 9;	break;
			}
			break;
			

		// Lugar 19
		case CABLE_MAZO_082:
			switch( direccion )
			{
				case 1:			direccion = 15;	break;
				case 2:			direccion = 15;	break;
				case 3:			direccion = 8;	break;
				case 4:			direccion = 8;	break;
				case 5:			direccion = 9;	break;
				case 6:			direccion = 9;	break;
				case 7:			direccion = 11;	break;
			}
			break;
			
			
		// Lugar 20
		case CABLE_MAZO_096:
			switch( direccion )
			{
				case 1:			direccion = 0;	break;
				case 2:			direccion = 1;	break;
				case 3:			direccion = 2;	break;
				case 4:			direccion = 3;	break;
				case 5:			direccion = 4;	break;
				case 6:			direccion = 5;	break;
				case 7:			direccion = 6;	break;
			}
			break;
			
			
		// Lugar 21
		case CABLE_MAZO_097_A:
			switch( direccion )
			{
				case 1: 	direccion = 7; 		break;		// Brown
				case 2: 	direccion = 2; 		break;		// Light blue
				case 3: 	direccion = 6; 		break;		// Pink
				case 4: 	direccion = 3; 		break;		// Orange
				case 5: 	direccion = 4; 		break;		// Yellow
				case 6: 	direccion = 0; 		break;		// Green
				case 7: 	direccion = 5; 		break;		// Blue
				
				case 8: 	direccion = 8; 		break;		// Violet
				case 9: 	direccion = 11; 	break;		// Red
				case 10: 	direccion = 14; 	break;		// Shield
				case 11: 	direccion = 12; 	break;		// Wire
				case 12: 	direccion = 9; 		break;		// Red - Peltier
				case 13: 	direccion = 0; 		break;		// Green
				case 14: 	direccion = 10; 	break;		// Black - Peltier
				
				case 15: 	direccion = 1; 		break;		// Green
				case 16: 	direccion = 13; 	break;		// Black - Peltier
			}
			break;
			
		case CABLE_MAZO_097_B:
			switch( direccion )
			{
				case 1: 	direccion = 11; 		break;		// Brown
				case 2: 	direccion = 6; 			break;		// Light blue
				case 3: 	direccion = 4; 			break;		// Pink
				case 4: 	direccion = 12; 		break;		// Orange
				case 5: 	direccion = 7; 			break;		// Yellow
				case 6: 	direccion = 5; 			break;		// Green
				case 7: 	direccion = 14; 		break;		// Blue
			}
			break;
			
	
		// Lugar 23
		case CABLE_MAZO_032:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;		// Brown
				case 2: 	direccion = 1; 		break;		// Light blue
				case 3: 	direccion = 2; 		break;		// Pink
				case 4: 	direccion = 3; 		break;		// Orange
				case 5: 	direccion = 4; 		break;		// Yellow
				case 6: 	direccion = 5; 		break;		// Green
				case 7: 	direccion = 6; 		break;		// Blue
				
				case 8: 	direccion = 10; 	break;		// Violet
				case 9: 	direccion = 9; 		break;		// Red
				case 10: 	direccion = 8; 		break;		// Shield
				case 11: 	direccion = 15; 	break;		// Wire
				case 12: 	direccion = 11; 	break;		// Red - Peltier
				case 13: 	direccion = 5; 		break;		// Green
				case 14: 	direccion = 12; 	break;		// Black - Peltier
			}
			break;
				
		case CABLE_MAZO_066:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;		// Brown
				case 2: 	direccion = 1; 		break;		// Light blue
				case 3: 	direccion = 2; 		break;		// Pink
				case 4: 	direccion = 3; 		break;		// Orange
				case 5: 	direccion = 4; 		break;		// Yellow
				case 6: 	direccion = 5; 		break;		// Green
				case 7: 	direccion = 6; 		break;		// Blue
				
				case 8: 	direccion = 10; 	break;		// Violet
				case 9: 	direccion = 9; 		break;		// Red
				case 10: 	direccion = 8; 		break;		// Shield
				case 11: 	direccion = 15; 	break;		// Wire
				case 12: 	direccion = 11; 	break;		// Red - Peltier
				case 13: 	direccion = 5; 		break;		// Green
				case 14: 	direccion = 12; 	break;		// Black - Peltier
			}
			break;
			
			
		// Lugar 24
		case CABLE_MAZO_111:
			switch( direccion )
			{
				case 1: 	direccion = 11; 		break;		// Brown
				case 2: 	direccion = 6; 			break;		// Light blue
				case 3: 	direccion = 4; 			break;		// Pink
				case 4: 	direccion = 12; 		break;		// Orange
				case 5: 	direccion = 7; 			break;		// Yellow
				case 6: 	direccion = 5; 			break;		// Green
				case 7: 	direccion = 14; 		break;		// Blue
			}
			break;
			
			
		// Lugar 25
		case CABLE_MAZO_035:
			switch( direccion )
			{
				case 1: 	direccion = 11; 		break;		// Brown
				case 2: 	direccion = 12; 		break;		// Light blue
				case 3: 	direccion = 13; 		break;		// Pink
				case 4: 	direccion = 14; 		break;		// Orange
			}
			break;
			
			
		// Lugar 26
		case CABLE_MAZO_034:
			switch( direccion )
			{
				case 1: 	direccion = 0; 		break;		// Red
				case 2: 	direccion = 1; 		break;		// Green
				case 3: 	direccion = 2; 		break;		// Negro
				case 4: 	direccion = 3; 		break;		// Marron
				case 5: 	direccion = 4; 		break;		// Blanco
				case 6:		direccion = 5; 		break;		// Malla del marron
				case 7:		direccion = 6; 		break;		// Malla del negro y del blanco
			}
			break;
			
			
		// Lugar para el cable de telefono
		case CABLE_MAZO_000:
			switch( direccion )
			{
				case 1:		direccion = 0;	break;
				case 2:		direccion = 1;	break;
				case 3:		direccion = 2;	break;
				case 4:		direccion = 3;	break;
			}
			break;
			
	}
	
	// Se modifican los pines para colocar la direccion solicitada
	auxiliar = direccion;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S0 de la placa
		S0_MULTIPLEXOR_PLACA_PIN = 1;
	}
	else
	{
		// Se borra el pin S0 de la placa
		S0_MULTIPLEXOR_PLACA_PIN = 0;
	}
	
	auxiliar = direccion >> 1;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S1 de la placa
		S1_MULTIPLEXOR_PLACA_PIN = 1;
	}
	else
	{
		// Se borra el pin S1 de la placa
		S1_MULTIPLEXOR_PLACA_PIN = 0;
	}
	
	auxiliar = direccion >> 2;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S2 de la placa
		S2_MULTIPLEXOR_PLACA_PIN = 1;
	}
	else
	{
		// Se borra el pin S2 de la placa
		S2_MULTIPLEXOR_PLACA_PIN = 0;
	}
	
	auxiliar = direccion >> 3;
	auxiliar &= 0x01;
	if( auxiliar > 0 )
	{
		// Se setea el pin S3 de la placa
		S3_MULTIPLEXOR_PLACA_PIN = 1;
	}
	else
	{
		// Se borra el pin S3 de la placa
		S3_MULTIPLEXOR_PLACA_PIN = 0;
	}
}



resultadoPernos VerificarMatrizObtenida ( char target )
{
	// Variables auxiliares
	char indiceContacto;
	
	// Indicadores de error
	bool	hayPinesAbiertos = FALSE;
	bool	hayPinesEnCorto = FALSE;
	bool	hayPinesAlternados = FALSE;
	bool	cableSinEmpalmes = TRUE;
	
	// Contador de contactos para los cables simples, que no tengan ningun pin en empalmado con otro
	char	totalDeContactos;
	
	// Variables para verificar el cable de alimentacion de 220V (109)
	char	a, b, c;
	
	// Se borran los contadores
	hayPinesAbiertos = FALSE;
	hayPinesEnCorto = FALSE;
	hayPinesAlternados = FALSE;
	
	
	// Se selecciona el total de contactos en funcion del target
	switch( target )
	{
		// Lugar 1
		case CABLE_MAZO_001:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_001;
			cableSinEmpalmes = TRUE;
			break;
		case CABLE_MAZO_041:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_041;
			cableSinEmpalmes = TRUE;
			break;
		case CABLE_MAZO_105:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_105;
			cableSinEmpalmes = TRUE;
			break;
			
		// Lugar 2
		case CABLE_MAZO_002:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_002;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_042:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_042;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_072:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_072;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_106:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_106;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 3
		case CABLE_MAZO_003:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_003;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_004:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_004;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_005:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_005;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_026:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_026;
			cableSinEmpalmes = TRUE;
			break;
				
		case CABLE_MAZO_043:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_043;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_044:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_044;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_060:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_060;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_073:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_073;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_074:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_074;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_110:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_110;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 4
		case CABLE_MAZO_006:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_006;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_007:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_007;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_045:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_045;
			cableSinEmpalmes = TRUE;
			break;
				
		case CABLE_MAZO_046:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_046;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_075:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_075;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_107:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_107;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 5
		case CABLE_MAZO_012:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_012;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_051:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_051;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 6
		case CABLE_MAZO_025:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_025;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_071:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_071;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_083:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_083;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 7
		case CABLE_MAZO_047:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_047;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_076:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_076;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 8
		case CABLE_MAZO_011:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_011;
			cableSinEmpalmes = TRUE;
			break;
		case CABLE_MAZO_050:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_050;
			cableSinEmpalmes = TRUE;
			break;
		case CABLE_MAZO_108:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_108;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 9
		case CABLE_MAZO_023:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_023;
			cableSinEmpalmes = TRUE;
			break;
		case CABLE_MAZO_081:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_081;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 10
		case CABLE_MAZO_021:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_021;
			cableSinEmpalmes = TRUE;
			break;
		case CABLE_MAZO_079:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_079;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 11
		case CABLE_MAZO_031:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_031;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 12
		case CABLE_MAZO_038:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_038;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 13
		case CABLE_MAZO_078:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_078;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 14
		case CABLE_MAZO_030:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_030;
			cableSinEmpalmes = FALSE;
			break;
			
			
		// Lugar 15
		case CABLE_MAZO_090:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_090;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 16
		case CABLE_MAZO_009:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_009;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_048:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_048;
			cableSinEmpalmes = TRUE;
			break;
			
		case CABLE_MAZO_077:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_077;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 17
		case CABLE_MAZO_022:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_022;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 18
		case CABLE_MAZO_024:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_024;
			cableSinEmpalmes = FALSE;
			break;
		case CABLE_MAZO_109:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_109;
			cableSinEmpalmes = FALSE;
			break;
			
			
		// Lugar 19
		case CABLE_MAZO_082:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_082;
			cableSinEmpalmes = FALSE;
			break;
			
			
		// Lugar 20
		case CABLE_MAZO_096:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_096;
			cableSinEmpalmes = TRUE;
			break;

			
		// Lugar 21
		case CABLE_MAZO_097_A:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_097_A;
			cableSinEmpalmes = FALSE;
			break;
		case CABLE_MAZO_097_B:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_097_B;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 23
		case CABLE_MAZO_032:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_032;
			cableSinEmpalmes = FALSE;
			break;
		case CABLE_MAZO_066:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_066;
			cableSinEmpalmes = FALSE;
			break;
			
			
		// Lugar 24
		case CABLE_MAZO_111:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_111;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 25
		case CABLE_MAZO_035:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_035;
			cableSinEmpalmes = TRUE;
			break;
			
			
		// Lugar 26
		case CABLE_MAZO_034:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_034;
			cableSinEmpalmes = FALSE;
			break;
			
			
		// Lugar para el cable de telefono
		case CABLE_MAZO_000:
			totalDeContactos = TOTAL_DE_PERNOS_CABLE_MAZO_000;
			cableSinEmpalmes = TRUE;
			break;
			
	}
	
	// Si es un cable simple, solo se deberia tener una matriz diagonal
	if( cableSinEmpalmes == TRUE )
	{
		for( indiceContacto = 1; indiceContacto <= totalDeContactos; indiceContacto++ )
		{
			if( pernos[ indiceContacto ].totalDeContactos == 0 )
			{
				hayPinesAbiertos = TRUE;
				condicionDelPerno[ indiceContacto ] = PERNO_ABIERTO;
			}
			else
			{
				if( pernos[ indiceContacto ].totalDeContactos > 1 )
				{
					hayPinesEnCorto = TRUE;
					condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
				}
				else
				{
					if( pernosUbicacion[ indiceContacto ] != indiceContacto )
					{
						hayPinesAlternados = TRUE;
						condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
					}
					else
					{
						condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
					}
				}
			}
		}
		
		if( hayPinesAbiertos == TRUE )
			return( PERNO_ABIERTO );
		else
			if( hayPinesEnCorto == TRUE )
				return( PERNO_CORTO_CIRCUITO );
			else
				if( hayPinesAlternados == TRUE )
					return( PERNO_SIN_ALTERNANCIA );
				else
				return( PERNO_SIN_ERRORES );
	}
	
	// Los cables especiales, que tienen empalmes que se detectan como cortos, se prueban de otra forma
	else
	{
		switch( target )
		{
			// El pin 6 esta empalmado con el pin 13
			case CABLE_MAZO_032:
				for( indiceContacto = 1; indiceContacto <= totalDeContactos; indiceContacto++ )
				{
					// Primero se revisa que no haya ningun perno abierto
					if( pernos[ indiceContacto ].totalDeContactos == 0 )
					{
						hayPinesAbiertos = TRUE;
						condicionDelPerno[ indiceContacto ] = PERNO_ABIERTO;
					}
					else
					{
						// Luego se revisa que no haya pernos en cortociruito
						if( pernos[ indiceContacto ].totalDeContactos > 1 )
						{
							// Para este cable, el 6 y el 13 deberian tener contacto
							if( indiceContacto == 6 || indiceContacto == 13 )
							{
								// Si hay mas de 2 contactos, estan en corto con otro cable
								if( pernos[ indiceContacto ].totalDeContactos > 2 )
								{
									hayPinesEnCorto = TRUE;
									condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
								}
								else
								{
									// En este caso, solo hay 2 cables empalmados. Resta ver que cada uno este en su posicion correcta
									if( pernos[ indiceContacto ].contactos[ indiceContacto ] == HAY_CONTINUIDAD )
									{
										condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
									}
									// Si no, esta fuera de lugar
									else
									{
										hayPinesAlternados = TRUE;
										condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
									}
								}
							}
							else
							{
								hayPinesEnCorto = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
							}
						}
						else
						{
							// Si el perno tiene un solo contacto, se revisa que este en el lugar correcto
							if( pernos[ indiceContacto ].contactos[ indiceContacto ] == HAY_CONTINUIDAD )
							{
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
							}
							else
							{
								hayPinesAlternados = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
							}
						}
					}
				}
				
				if( hayPinesAbiertos == TRUE )
					return( PERNO_ABIERTO );
				else
					if( hayPinesEnCorto == TRUE )
						return( PERNO_CORTO_CIRCUITO );
					else
						if( hayPinesAlternados == TRUE )
							return( PERNO_SIN_ALTERNANCIA );
						else
							return( PERNO_SIN_ERRORES );
			break;
			
			
			case CABLE_MAZO_066:
				for( indiceContacto = 1; indiceContacto <= totalDeContactos; indiceContacto++ )
				{
					// Primero se revisa que no haya ningun perno abierto
					if( pernos[ indiceContacto ].totalDeContactos == 0 )
					{
						hayPinesAbiertos = TRUE;
						condicionDelPerno[ indiceContacto ] = PERNO_ABIERTO;
					}
					else
					{
						// Luego se revisa que no haya pernos en cortociruito
						if( pernos[ indiceContacto ].totalDeContactos > 1 )
						{
							// Para este cable, el 6 y el 13 deberian tener contacto
							if( indiceContacto == 6 || indiceContacto == 13 )
							{
								// Si hay mas de 2 contactos, estan en corto con otro cable
								if( pernos[ indiceContacto ].totalDeContactos > 2 )
								{
									hayPinesEnCorto = TRUE;
									condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
								}
								else
								{
									// En este caso, solo hay 2 cables empalmados. Resta ver que cada uno este en su posicion correcta
									if( pernos[ indiceContacto ].contactos[ indiceContacto ] == HAY_CONTINUIDAD )
									{
										condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
									}
									// Si no, esta fuera de lugar
									else
									{
										hayPinesAlternados = TRUE;
										condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
									}
								}
							}
							else
							{
								hayPinesEnCorto = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
							}
						}
						else
						{
							// Si el perno tiene un solo contacto, se revisa que este en el lugar correcto
							if( pernos[ indiceContacto ].contactos[ indiceContacto ] == HAY_CONTINUIDAD )
							{
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
							}
							else
							{
								hayPinesAlternados = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
							}
						}
					}
				}
				
				if( hayPinesAbiertos == TRUE )
					return( PERNO_ABIERTO );
				else
					if( hayPinesEnCorto == TRUE )
						return( PERNO_CORTO_CIRCUITO );
					else
						if( hayPinesAlternados == TRUE )
							return( PERNO_SIN_ALTERNANCIA );
						else
							return( PERNO_SIN_ERRORES );
			break;

			
			case CABLE_MAZO_024:

				for( indiceContacto = 1; indiceContacto <= 9; indiceContacto++ )
				{
					switch( indiceContacto )
					{
						case 1:		a = 1; b = 4; c = 7;	break;
						case 2:		a = 1; b = 4; c = 7;	break;
						case 3:		a = 1; b = 4; c = 7;	break;
						
						case 4:		a = 4; b = 1; c = 7;	break;
						case 5:		a = 4; b = 1; c = 7;	break;
						case 6:		a = 4; b = 1; c = 7;	break;
						
						case 7:		a = 7; b = 4; c = 1;	break;
						case 8:		a = 7; b = 4; c = 1;	break;
						case 9:		a = 7; b = 4; c = 1;	break;
					}
					
					// 0
					if( pernos[ a ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
					{
						// 0
						if( pernos[ b ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
						{
							// 0
							if( pernos[ c ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
							{
								hayPinesAbiertos = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_ABIERTO;
							}
							// 1
							else
							{
								hayPinesAlternados = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
							}
						}
						// 1
						else
						{
							// 0
							if( pernos[ c ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
							{
								hayPinesAlternados = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
							}
							// 1
							else
							{
								hayPinesEnCorto = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
							}
						}
					}
					
					// 1
					else
					{
						// 0
						if( pernos[ b ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
						{
							// 0
							if( pernos[ c ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
							{
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
							}
							// 1
							else
							{
								hayPinesEnCorto = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
							}
						}
						// 1
						else
						{
							// 0
							// 1
							hayPinesEnCorto = TRUE;
							condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
						}
					}
				}
				
				if( hayPinesAbiertos == TRUE )
					return( PERNO_ABIERTO );
				else
					if( hayPinesEnCorto == TRUE )
						return( PERNO_CORTO_CIRCUITO );
					else
						if( hayPinesAlternados == TRUE )
							return( PERNO_SIN_ALTERNANCIA );
						else
							return( PERNO_SIN_ERRORES );
				
				break;
			
				
			case CABLE_MAZO_109:
				
				for( indiceContacto = 1; indiceContacto <= 7; indiceContacto++ )
				{
					switch( indiceContacto )
					{
						case 1:		a = 1; b = 3; c = 5;	break;
						case 2:		a = 1; b = 3; c = 5;	break;
						
						case 3:		a = 3; b = 1; c = 5;	break;
						case 4:		a = 3; b = 1; c = 5;	break;
						
						case 5:		a = 5; b = 3; c = 1;	break;
						case 6:		a = 5; b = 3; c = 1;	break;
						case 7:		a = 5; b = 3; c = 1;	break;
					}
					
					// 0
					if( pernos[ a ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
					{
						// 0
						if( pernos[ b ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
						{
							// 0
							if( pernos[ c ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
							{
								hayPinesAbiertos = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_ABIERTO;
							}
							// 1
							else
							{
								hayPinesAlternados = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
							}
						}
						// 1
						else
						{
							// 0
							if( pernos[ c ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
							{
								hayPinesAlternados = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
							}
							// 1
							else
							{
								hayPinesEnCorto = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
							}
						}
					}
					
					// 1
					else
					{
						// 0
						if( pernos[ b ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
						{
							// 0
							if( pernos[ c ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
							{
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
							}
							// 1
							else
							{
								hayPinesEnCorto = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
							}
						}
						// 1
						else
						{
							// 0
							// 1
							hayPinesEnCorto = TRUE;
							condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
						}
					}
				}
				
				if( hayPinesAbiertos == TRUE )
					return( PERNO_ABIERTO );
				else
					if( hayPinesEnCorto == TRUE )
						return( PERNO_CORTO_CIRCUITO );
					else
						if( hayPinesAlternados == TRUE )
							return( PERNO_SIN_ALTERNANCIA );
						else
							return( PERNO_SIN_ERRORES );
				
				break;
				
				
			case CABLE_MAZO_030:
				
				for( indiceContacto = 1; indiceContacto <= 6; indiceContacto++ )
				{
					switch( indiceContacto )
					{
						case 1:		a = 1; b = 4;	break;
						case 2:		a = 1; b = 4;	break;
						case 3:		a = 2; b = 4;	break;
						
						case 4:		a = 4; b = 1;	break;
						case 5:		a = 4; b = 1;	break;
						case 6:		a = 5; b = 1;	break;
					}
					
					// 0
					if( pernos[ a ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
					{
						// 0
						if( pernos[ b ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
						{
							hayPinesAbiertos = TRUE;
							condicionDelPerno[ indiceContacto ] = PERNO_ABIERTO;
						}
						// 1
						else
						{
							hayPinesAlternados = TRUE;
							condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
						}
					}
					
					// 1
					else
					{
						// 0
						if( pernos[ b ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
						{
							condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
						}
						// 1
						else
						{
							hayPinesEnCorto = TRUE;
							condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
						}
					}
				}
				
				if( hayPinesAbiertos == TRUE )
					return( PERNO_ABIERTO );
				else
					if( hayPinesEnCorto == TRUE )
						return( PERNO_CORTO_CIRCUITO );
					else
						if( hayPinesAlternados == TRUE )
							return( PERNO_SIN_ALTERNANCIA );
						else
							return( PERNO_SIN_ERRORES );
				
				break;
				
				
			case CABLE_MAZO_082:
				
				for( indiceContacto = 1; indiceContacto <= 6; indiceContacto++ )
				{
					switch( indiceContacto )
					{
						case 1:		a = 1; b = 3; c = 5;	break;
						case 2:		a = 1; b = 3; c = 5;	break;
						
						case 3:		a = 3; b = 1; c = 5;	break;
						case 4:		a = 3; b = 1; c = 5;	break;
						
						case 5:		a = 5; b = 3; c = 1;	break;
						case 6:		a = 5; b = 3; c = 1;	break;
					}
					
					// 0
					if( pernos[ a ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
					{
						// 0
						if( pernos[ b ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
						{
							// 0
							if( pernos[ c ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
							{
								hayPinesAbiertos = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_ABIERTO;
							}
							// 1
							else
							{
								hayPinesAlternados = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
							}
						}
						// 1
						else
						{
							// 0
							if( pernos[ c ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
							{
								hayPinesAlternados = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
							}
							// 1
							else
							{
								hayPinesEnCorto = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
							}
						}
					}
					
					// 1
					else
					{
						// 0
						if( pernos[ b ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
						{
							// 0
							if( pernos[ c ].contactos[ indiceContacto ] == NO_HAY_CONTINUIDAD )
							{
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
							}
							// 1
							else
							{
								hayPinesEnCorto = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
							}
						}
						// 1
						else
						{
							// 0
							// 1
							hayPinesEnCorto = TRUE;
							condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
						}
					}
				}
				
				
				if( pernos[ 7 ].totalDeContactos == 0 )
				{
					hayPinesAbiertos = TRUE;
					condicionDelPerno[ indiceContacto ] = PERNO_ABIERTO;
				}
				else
				{
					if( pernos[ 7 ].totalDeContactos > 1 )
					{
						hayPinesEnCorto = TRUE;
						condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
					}
					else
						condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
				}
				
				
				if( hayPinesAbiertos == TRUE )
					return( PERNO_ABIERTO );
				else
					if( hayPinesEnCorto == TRUE )
						return( PERNO_CORTO_CIRCUITO );
					else
						if( hayPinesAlternados == TRUE )
							return( PERNO_SIN_ALTERNANCIA );
						else
							return( PERNO_SIN_ERRORES );
				
				break;
			
				
			case CABLE_MAZO_097_A:
				for( indiceContacto = 1; indiceContacto <= totalDeContactos; indiceContacto++ )
				{
					// Primero se revisa que no haya ningun perno abierto
					if( pernos[ indiceContacto ].totalDeContactos == 0 )
					{
						hayPinesAbiertos = TRUE;
						condicionDelPerno[ indiceContacto ] = PERNO_ABIERTO;
					}
					else
					{
						// Luego se revisa que no haya pernos en cortociruito
						if( pernos[ indiceContacto ].totalDeContactos > 1 )
						{
							// Para este cable, el 6 y el 13 deberian tener contacto
							if( indiceContacto == 6 || indiceContacto == 13 )
							{
								// Si hay mas de 2 contactos, estan en corto con otro cable
								if( pernos[ indiceContacto ].totalDeContactos > 2 )
								{
									hayPinesEnCorto = TRUE;
									condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
								}
								else
								{
									// En este caso, solo hay 2 cables empalmados. Resta ver que cada uno este en su posicion correcta
									if( pernos[ indiceContacto ].contactos[ indiceContacto ] == HAY_CONTINUIDAD )
									{
										condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
									}
									// Si no, esta fuera de lugar
									else
									{
										hayPinesAlternados = TRUE;
										condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
									}
								}
							}
							else
							{
								hayPinesEnCorto = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_CORTO_CIRCUITO;
							}
						}
						else
						{
							// Si el perno tiene un solo contacto, se revisa que este en el lugar correcto
							if( pernos[ indiceContacto ].contactos[ indiceContacto ] == HAY_CONTINUIDAD )
							{
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ERRORES;
							}
							else
							{
								hayPinesAlternados = TRUE;
								condicionDelPerno[ indiceContacto ] = PERNO_SIN_ALTERNANCIA;
							}
						}
					}
				}
				
				if( hayPinesAbiertos == TRUE )
					return( PERNO_ABIERTO );
				else
					if( hayPinesEnCorto == TRUE )
						return( PERNO_CORTO_CIRCUITO );
					else
						if( hayPinesAlternados == TRUE )
							return( PERNO_SIN_ALTERNANCIA );
						else
							return( PERNO_SIN_ERRORES );
				
				break;
				
				
			case CABLE_MAZO_034:

				// Red
				if( pernos[ 1 ].totalDeContactos == 0 )
				{
					hayPinesAbiertos = TRUE;
					condicionDelPerno[ 1 ] = PERNO_ABIERTO;
				}
				else
				{
					if( pernos[ 1 ].totalDeContactos > 1 )
					{
						hayPinesEnCorto = TRUE;
						condicionDelPerno[ 1 ] = PERNO_CORTO_CIRCUITO;
					}
					else
					{
						if( pernos[ 1 ].contactos[ 1 ] == HAY_CONTINUIDAD )
						{
							condicionDelPerno[ 1 ] = PERNO_SIN_ERRORES;
						}
						else
						{
							hayPinesAlternados = TRUE;
							condicionDelPerno[ 1 ] = PERNO_SIN_ALTERNANCIA;
						}
					}
				}
				
				// Green
				if( pernos[ 2 ].totalDeContactos == 0 )
				{
					hayPinesAbiertos = TRUE;
					condicionDelPerno[ 2 ] = PERNO_ABIERTO;
				}
				else
				{
					if( pernos[ 2 ].totalDeContactos > 1 )
					{
						hayPinesEnCorto = TRUE;
						condicionDelPerno[ 2 ] = PERNO_CORTO_CIRCUITO;
					}
					else
					{
						if( pernos[ 2 ].contactos[ 2 ] == HAY_CONTINUIDAD )
						{
							condicionDelPerno[ 2 ] = PERNO_SIN_ERRORES;
						}
						else
						{
							hayPinesAlternados = TRUE;
							condicionDelPerno[ 2 ] = PERNO_SIN_ALTERNANCIA;
						}
					}
				}
				
				// Black
				if( pernos[ 3 ].totalDeContactos == 0 )
				{
					hayPinesAbiertos = TRUE;
					condicionDelPerno[ 3 ] = PERNO_ABIERTO;
				}
				else
				{
					if( pernos[ 3 ].totalDeContactos > 3 )
					{
						hayPinesEnCorto = TRUE;
						condicionDelPerno[ 3 ] = PERNO_CORTO_CIRCUITO;
					}
					else
					{
						if( pernos[ 3 ].contactos[ 3 ] == HAY_CONTINUIDAD )
						{
							condicionDelPerno[ 3 ] = PERNO_SIN_ERRORES;
						}
						else
						{
							hayPinesAlternados = TRUE;
							condicionDelPerno[ 3 ] = PERNO_SIN_ALTERNANCIA;
						}
					}
				}
				
				// Brown
				if( pernos[ 4 ].totalDeContactos == 0 )
				{
					hayPinesAbiertos = TRUE;
					condicionDelPerno[ 4 ] = PERNO_ABIERTO;
				}
				else
				{
					if( pernos[ 4 ].totalDeContactos > 3 )
					{
						hayPinesEnCorto = TRUE;
						condicionDelPerno[ 4 ] = PERNO_CORTO_CIRCUITO;
					}
					else
					{
						if( pernos[ 4 ].contactos[ 4 ] == HAY_CONTINUIDAD )
						{
							condicionDelPerno[ 4 ] = PERNO_SIN_ERRORES;
						}
						else
						{
							hayPinesAlternados = TRUE;
							condicionDelPerno[ 4 ] = PERNO_SIN_ALTERNANCIA;
						}
					}
				}
				
				// White
				if( pernos[ 5 ].totalDeContactos == 0 )
				{
					hayPinesAbiertos = TRUE;
					condicionDelPerno[ 5 ] = PERNO_ABIERTO;
				}
				else
				{
					if( pernos[ 5 ].totalDeContactos > 3 )
					{
						hayPinesEnCorto = TRUE;
						condicionDelPerno[ 5 ] = PERNO_CORTO_CIRCUITO;
					}
					else
					{
						if( pernos[ 5 ].contactos[ 5 ] == HAY_CONTINUIDAD )
						{
							condicionDelPerno[ 5 ] = PERNO_SIN_ERRORES;
						}
						else
						{
							hayPinesAlternados = TRUE;
							condicionDelPerno[ 5 ] = PERNO_SIN_ALTERNANCIA;
						}
					}
				}
				
				// Mallas
				if( pernos[ 6 ].totalDeContactos == 0 )
				{
					hayPinesAbiertos = TRUE;
					condicionDelPerno[ 6 ] = PERNO_ABIERTO;
					condicionDelPerno[ 7 ] = PERNO_ABIERTO;
				}
				else
				{
					if( pernos[ 6 ].totalDeContactos > 2 )
					{
						hayPinesEnCorto = TRUE;
						condicionDelPerno[ 6 ] = PERNO_CORTO_CIRCUITO;
						condicionDelPerno[ 7 ] = PERNO_CORTO_CIRCUITO;
					}
					else
					{
						// Malla del maron
						if( pernos[ 6 ].contactos[ 6 ] == HAY_CONTINUIDAD )
						{
							condicionDelPerno[ 6 ] = PERNO_SIN_ERRORES;
						}
						else
						{
							hayPinesAlternados = TRUE;
							condicionDelPerno[ 6 ] = PERNO_SIN_ALTERNANCIA;
						}
						
						// Malla del negro y del blanco
						if( pernos[ 6 ].contactos[ 7 ] == HAY_CONTINUIDAD )
						{
							condicionDelPerno[ 7 ] = PERNO_SIN_ERRORES;
						}
						else
						{
							hayPinesAlternados = TRUE;
							condicionDelPerno[ 7 ] = PERNO_SIN_ALTERNANCIA;
						}
					}
				}
				
				
				if( hayPinesAbiertos == TRUE )
					return( PERNO_ABIERTO );
				else
					if( hayPinesEnCorto == TRUE )
						return( PERNO_CORTO_CIRCUITO );
					else
						if( hayPinesAlternados == TRUE )
							return( PERNO_SIN_ALTERNANCIA );
						else
							return( PERNO_SIN_ERRORES );
				
				break;
				
		}
	}
	
}




void RenumerarContactos ( char target )
{
	// La idea es recibir la matriz despues de la recepcion y modificar los numeros segun los contactos indicados en el plano
	resultadoPernos			condicionDelPernoAuxiliar[ TOTAL_DE_CONTACTOS_MULTIPLEXORES + 1 ];
	char					pernosUbicacionAuxiliar[ TOTAL_DE_CONTACTOS_MULTIPLEXORES + 1 ];
	char					contador;
	
	// Se copian las variables y se borran las originales
	for( contador = 1; contador <= TOTAL_DE_CONTACTOS_MULTIPLEXORES; contador++ )
	{
		condicionDelPernoAuxiliar[ contador ] = condicionDelPerno[ contador ];
		pernosUbicacionAuxiliar[ contador ] = pernosUbicacion[ contador ];
		condicionDelPerno[ contador ] = PERNO_SIN_ERRORES;
		pernosUbicacion[ contador ] = 0;
	}
	
	// Se reacomodan los contactos segun el plano
	switch( target )
	{
		// Lugar 1
		case CABLE_MAZO_001:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
		case CABLE_MAZO_041:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
		case CABLE_MAZO_105:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
			
		// Lugar 2
		case CABLE_MAZO_002:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_042:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_072:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_106:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
			
		// Lugar 3
		case CABLE_MAZO_003:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_004:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_005:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
		case CABLE_MAZO_026:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_043:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_044:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_060:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_073:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_074:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
		case CABLE_MAZO_110:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
			
		// Lugar 4
		case CABLE_MAZO_006:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			break;
			
		case CABLE_MAZO_007:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
		case CABLE_MAZO_045:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			break;
			
		case CABLE_MAZO_046:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
		case CABLE_MAZO_075:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			break;
			
		case CABLE_MAZO_107:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
			
		// Lugar 5
		case CABLE_MAZO_012:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
		case CABLE_MAZO_051:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
			
		// Lugar 6
		case CABLE_MAZO_025:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
		case CABLE_MAZO_071:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
		case CABLE_MAZO_083:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
			
		// Lugar 7
		case CABLE_MAZO_047:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			break;
			
		case CABLE_MAZO_076:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			break;
			
			
		// Lugar 8
		case CABLE_MAZO_011:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_050:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;
			
		case CABLE_MAZO_108:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 1 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 1 ];
			break;

			
		// Lugar 9
		case CABLE_MAZO_023:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
		case CABLE_MAZO_081:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
			
		// Lugar 10
		case CABLE_MAZO_021:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
		case CABLE_MAZO_079:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
			
		// Lugar 11
		case CABLE_MAZO_031:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
			
		// Lugar 12
		case CABLE_MAZO_038:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
			
		// Lugar 13
		case CABLE_MAZO_078:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
			
		// Lugar 14
		case CABLE_MAZO_030:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			break;
			
			
		// Lugar 15
		case CABLE_MAZO_090:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
			
		// Lugar 16
		case CABLE_MAZO_009:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			condicionDelPerno[ 8 ] = condicionDelPernoAuxiliar[ 8 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			pernosUbicacion[ 8 ] = pernosUbicacionAuxiliar[ 8 ];
			break;
			
		case CABLE_MAZO_048:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			condicionDelPerno[ 8 ] = condicionDelPernoAuxiliar[ 8 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			pernosUbicacion[ 8 ] = pernosUbicacionAuxiliar[ 8 ];
			break;
			
		case CABLE_MAZO_077:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			condicionDelPerno[ 8 ] = condicionDelPernoAuxiliar[ 8 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			pernosUbicacion[ 8 ] = pernosUbicacionAuxiliar[ 8 ];
			break;
			
			
		// Lugar 17
		case CABLE_MAZO_022:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			break;
			
			
		// Lugar 18
		case CABLE_MAZO_024:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 7 ];		// OK
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 4 ];		// OK
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 1 ];		// OK
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 8 ];		// OK
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 2 ];		// OK
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 9 ];		// OK
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 3 ];		// OK
			condicionDelPerno[ 8 ] = condicionDelPernoAuxiliar[ 5 ];		// OK
			condicionDelPerno[ 9 ] = condicionDelPernoAuxiliar[ 6 ];		// OK
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 7 ];			// OK
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 4 ];			// OK
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 1 ];			// OK
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 8 ];			// OK
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 2 ];			// OK
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 9 ];			// OK
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 3 ];			// OK
			pernosUbicacion[ 8 ] = pernosUbicacionAuxiliar[ 5 ];			// OK
			pernosUbicacion[ 9 ] = pernosUbicacionAuxiliar[ 6 ];			// OK
			break;
			
		case CABLE_MAZO_109:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 3 ];		// OK
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 1 ];		// OK
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 5 ];		// OK
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];		// OK
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 2 ];		// OK
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];		// OK
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];		// OK
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 3 ];			// OK
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 1 ];			// OK
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 5 ];			// OK
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];			// OK
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 2 ];			// OK
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];			// OK
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];			// OK
			break;
			
			
		// Lugar 19
		case CABLE_MAZO_082:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 3 ];		// OK
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 1 ];		// OK
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 4 ];		// OK
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 5 ];		// OK
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 6 ];		// OK
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 2 ];		// OK
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];		// OK
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 3 ];			// OK
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 1 ];			// OK
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 4 ];			// OK
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 5 ];			// OK
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 6 ];			// OK
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 2 ];			// OK
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];			// OK
			break;
			
			
		// Lugar 20
		case CABLE_MAZO_096:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			break;
			

		// Lugar 21
		case CABLE_MAZO_097_A:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			condicionDelPerno[ 8 ] = condicionDelPernoAuxiliar[ 8 ];
			condicionDelPerno[ 9 ] = condicionDelPernoAuxiliar[ 9 ];
			condicionDelPerno[ 10 ] = condicionDelPernoAuxiliar[ 10 ];
			condicionDelPerno[ 11 ] = condicionDelPernoAuxiliar[ 11 ];
			condicionDelPerno[ 12 ] = condicionDelPernoAuxiliar[ 12 ];
			condicionDelPerno[ 13 ] = condicionDelPernoAuxiliar[ 13 ];
			condicionDelPerno[ 14 ] = condicionDelPernoAuxiliar[ 14 ];
			condicionDelPerno[ 15 ] = condicionDelPernoAuxiliar[ 15 ];
			condicionDelPerno[ 16 ] = condicionDelPernoAuxiliar[ 16 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			pernosUbicacion[ 8 ] = pernosUbicacionAuxiliar[ 8 ];
			pernosUbicacion[ 9 ] = pernosUbicacionAuxiliar[ 9 ];
			pernosUbicacion[ 10 ] = pernosUbicacionAuxiliar[ 10 ];
			pernosUbicacion[ 11 ] = pernosUbicacionAuxiliar[ 11 ];
			pernosUbicacion[ 12 ] = pernosUbicacionAuxiliar[ 12 ];
			pernosUbicacion[ 13 ] = pernosUbicacionAuxiliar[ 13 ];
			pernosUbicacion[ 14 ] = pernosUbicacionAuxiliar[ 14 ];
			pernosUbicacion[ 15 ] = pernosUbicacionAuxiliar[ 15 ];
			pernosUbicacion[ 16 ] = pernosUbicacionAuxiliar[ 16 ];
			break;
			
		case CABLE_MAZO_097_B:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			break;
			
			
		// Lugar 23
		case CABLE_MAZO_032:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			condicionDelPerno[ 8 ] = condicionDelPernoAuxiliar[ 8 ];
			condicionDelPerno[ 9 ] = condicionDelPernoAuxiliar[ 9 ];
			condicionDelPerno[ 10 ] = condicionDelPernoAuxiliar[ 10 ];
			condicionDelPerno[ 11 ] = condicionDelPernoAuxiliar[ 11 ];
			condicionDelPerno[ 12 ] = condicionDelPernoAuxiliar[ 12 ];
			condicionDelPerno[ 13 ] = condicionDelPernoAuxiliar[ 13 ];
			condicionDelPerno[ 14 ] = condicionDelPernoAuxiliar[ 14 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			pernosUbicacion[ 8 ] = pernosUbicacionAuxiliar[ 8 ];
			pernosUbicacion[ 9 ] = pernosUbicacionAuxiliar[ 9 ];
			pernosUbicacion[ 10 ] = pernosUbicacionAuxiliar[ 10 ];
			pernosUbicacion[ 11 ] = pernosUbicacionAuxiliar[ 11 ];
			pernosUbicacion[ 12 ] = pernosUbicacionAuxiliar[ 12 ];
			pernosUbicacion[ 13 ] = pernosUbicacionAuxiliar[ 13 ];
			pernosUbicacion[ 14 ] = pernosUbicacionAuxiliar[ 14 ];
			break;
			
		case CABLE_MAZO_066:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			condicionDelPerno[ 8 ] = condicionDelPernoAuxiliar[ 8 ];
			condicionDelPerno[ 9 ] = condicionDelPernoAuxiliar[ 9 ];
			condicionDelPerno[ 10 ] = condicionDelPernoAuxiliar[ 10 ];
			condicionDelPerno[ 11 ] = condicionDelPernoAuxiliar[ 11 ];
			condicionDelPerno[ 12 ] = condicionDelPernoAuxiliar[ 12 ];
			condicionDelPerno[ 13 ] = condicionDelPernoAuxiliar[ 13 ];
			condicionDelPerno[ 14 ] = condicionDelPernoAuxiliar[ 14 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			pernosUbicacion[ 8 ] = pernosUbicacionAuxiliar[ 8 ];
			pernosUbicacion[ 9 ] = pernosUbicacionAuxiliar[ 9 ];
			pernosUbicacion[ 10 ] = pernosUbicacionAuxiliar[ 10 ];
			pernosUbicacion[ 11 ] = pernosUbicacionAuxiliar[ 11 ];
			pernosUbicacion[ 12 ] = pernosUbicacionAuxiliar[ 12 ];
			pernosUbicacion[ 13 ] = pernosUbicacionAuxiliar[ 13 ];
			pernosUbicacion[ 14 ] = pernosUbicacionAuxiliar[ 14 ];
			break;
			
			
		// Lugar 24
		case CABLE_MAZO_111:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			break;
			
			
		// Lugar 25
		case CABLE_MAZO_035:
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
			
		// Lugar 26
		case CABLE_MAZO_034:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			condicionDelPerno[ 5 ] = condicionDelPernoAuxiliar[ 5 ];
			condicionDelPerno[ 6 ] = condicionDelPernoAuxiliar[ 6 ];
			condicionDelPerno[ 7 ] = condicionDelPernoAuxiliar[ 7 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			pernosUbicacion[ 5 ] = pernosUbicacionAuxiliar[ 5 ];
			pernosUbicacion[ 6 ] = pernosUbicacionAuxiliar[ 6 ];
			pernosUbicacion[ 7 ] = pernosUbicacionAuxiliar[ 7 ];
			break;
			
			
		// Lugar para el cable de telefono
		case CABLE_MAZO_000:
			condicionDelPerno[ 1 ] = condicionDelPernoAuxiliar[ 1 ];
			condicionDelPerno[ 2 ] = condicionDelPernoAuxiliar[ 2 ];
			condicionDelPerno[ 3 ] = condicionDelPernoAuxiliar[ 3 ];
			condicionDelPerno[ 4 ] = condicionDelPernoAuxiliar[ 4 ];
			pernosUbicacion[ 1 ] = pernosUbicacionAuxiliar[ 1 ];
			pernosUbicacion[ 2 ] = pernosUbicacionAuxiliar[ 2 ];
			pernosUbicacion[ 3 ] = pernosUbicacionAuxiliar[ 3 ];
			pernosUbicacion[ 4 ] = pernosUbicacionAuxiliar[ 4 ];
			break;
			
	}
	
	return;
}
