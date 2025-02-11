/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "Vtpm1ovf.h"
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

/* User includes (#include below this line is not maintained by Processor Expert) */
#include "Display.h"
#include "DS18S20.h"
#include "Timer.h"







		/****************************************************************************************************************/
		/*			  						DEFINICIONES DE NUEVOS OBJETOS Y DATOS										*/
		/****************************************************************************************************************/

// Definicion de los estados para elegir o probar el cable
typedef enum saraza {
	DEFINIENDO_1_CIFRA = 0,
	DEFINIENDO_2_CIFRA,
	DEFINIENDO_3_CIFRA,
	PROBANDO_CABLE
} estadosPrograma;

// Definicion de los targets para las direcciones de los multiplexores
typedef enum targetMult {
	
	// Lugar 1
	CABLE_MAZO_001 = 0,
	CABLE_MAZO_041,
	CABLE_MAZO_105,
	
	// Lugar 2
	CABLE_MAZO_002,
	CABLE_MAZO_042,
	CABLE_MAZO_072,
	CABLE_MAZO_106,
	
	// Lugar 3
	CABLE_MAZO_003,
	CABLE_MAZO_004,
	CABLE_MAZO_005,
	CABLE_MAZO_026,
	CABLE_MAZO_043,
	CABLE_MAZO_044,
	CABLE_MAZO_060,
	CABLE_MAZO_073,
	CABLE_MAZO_074,
	CABLE_MAZO_110,
	
	// Lugar 4
	CABLE_MAZO_006,
	CABLE_MAZO_007,
	CABLE_MAZO_045,
	CABLE_MAZO_046,
	CABLE_MAZO_075,
	CABLE_MAZO_107,
	
	// Lugar 5
	CABLE_MAZO_012,
	CABLE_MAZO_051,
	
	// Lugar 6
	CABLE_MAZO_025,
	CABLE_MAZO_071,
	CABLE_MAZO_083,
	
	// Lugar 7
	CABLE_MAZO_047,
	CABLE_MAZO_076,
	
	// Lugar 8
	CABLE_MAZO_011,
	CABLE_MAZO_050,
	CABLE_MAZO_108,
	
	// Lugar 9
	CABLE_MAZO_023,
	CABLE_MAZO_081,
	
	// Lugar 10
	CABLE_MAZO_021,
	CABLE_MAZO_079,
	
	// Lugar 11
	CABLE_MAZO_031,

	// Lugar 12
	CABLE_MAZO_038,
	
	// Lugar 13
	CABLE_MAZO_078,
	
	// Lugar 14
	CABLE_MAZO_030,
	CABLE_MAZO_064,

	// Lugar 15
	CABLE_MAZO_090,

	// Lugar 16
	CABLE_MAZO_009,
	CABLE_MAZO_048,
	CABLE_MAZO_077,
	
	// Lugar 17
	CABLE_MAZO_022,
	
	// Lugar 18
	CABLE_MAZO_024,
	CABLE_MAZO_109,
	
	// Lugar 19
	CABLE_MAZO_082,
	
	// Lugar 20
	CABLE_MAZO_096,
	
	// Lugar 21
	CABLE_MAZO_097_A,
	CABLE_MAZO_097_B,
	
	// Lugar 23
	CABLE_MAZO_032,
	CABLE_MAZO_066,
	
	// Lugar 24
	CABLE_MAZO_111,
	
	// Lugar 25
	CABLE_MAZO_035,
	
	// Lugar 26
	CABLE_MAZO_034,
	
	// Lugar para el cable de telefono
	CABLE_MAZO_000
	
} targetMultiplexores;


// Definicion para interpretar mas facilmente los errores de los pernos
typedef enum erroresPernos {
	PERNO_SIN_ERRORES = 0,
	PERNO_ABIERTO,
	PERNO_CORTO_CIRCUITO,
	PERNO_CORTO_CIRCUITO_CARCAZA,
	PERNO_SIN_ALTERNANCIA,
	PERNO_CORTOCIRCUITO_CON_5V
} resultadoPernos;

// Definicion para interpretar mas facilmente los estados de los pulsadores
typedef enum teclaPresionada {
	TECLA_DERECHA_O_ABAJO = 0,
	TECLA_IZQUIERDA_O_ARRIBA,
	TECLA_ACEPTAR,
	TECLA_CANCELAR,
	TECLA_SELECCIONAR_MENU
} estadosTeclas;


// Definicion de la estructura para manejar los pernos como una matriz mas facil de interpretar
typedef struct	{
	bool	contactos[ 16 + 1 ];
	char	totalDeContactos;
} pernosObj;



		/****************************************************************************************************************/
		/*				  						DEFINICIONES DE LAS CONEXIONES											*/
		/****************************************************************************************************************/

// Defines para los LEDs
#define  	LED_ROJO_DIRECCION						PTCDD_PTCDD3
#define  	LED_ROJO_PIN							PTCD_PTCD3
#define  	LED_VERDE_DIRECCION						PTCDD_PTCDD5
#define  	LED_VERDE_PIN							PTCD_PTCD5

// Defines para los pulsadores
#define  	PULSADOR_S1_DIRECCION					PTEDD_PTEDD6
#define  	PULSADOR_S1_PIN							PTED_PTED6
#define  	PULSADOR_S1_PUERTO						PTED
#define		PULSADOR_S1_MASCARA						PTED_PTED6_MASK		// Es para leer solo el pin del pulsador

#define  	PULSADOR_S2_DIRECCION					PTEDD_PTEDD4
#define  	PULSADOR_S2_PIN							PTED_PTED4
#define  	PULSADOR_S2_PUERTO						PTED
#define		PULSADOR_S2_MASCARA						PTED_PTED4_MASK		// Es para leer solo el pin del pulsador

#define  	PULSADOR_S3_DIRECCION					PTEDD_PTEDD7
#define  	PULSADOR_S3_PIN							PTED_PTED7
#define  	PULSADOR_S3_PUERTO						PTED
#define		PULSADOR_S3_MASCARA						PTED_PTED7_MASK		// Es para leer solo el pin del pulsador

#define  	PULSADOR_S4_DIRECCION					PTEDD_PTEDD5
#define  	PULSADOR_S4_PIN							PTED_PTED05
#define  	PULSADOR_S4_PUERTO						PTED
#define		PULSADOR_S4_MASCARA						PTED_PTED5_MASK		// Es para leer solo el pin del pulsador



// Defines para los pines del multiplexor
#define		S0_MULTIPLEXOR_PLACA_PIN				PTDD_PTDD1			// PTD1
#define		S1_MULTIPLEXOR_PLACA_PIN				PTDD_PTDD2			// PTD2
#define		S2_MULTIPLEXOR_PLACA_PIN				PTBD_PTBD3			// PTB3
#define		S3_MULTIPLEXOR_PLACA_PIN				PTDD_PTDD0			// PTD0
#define		E_MULTIPLEXOR_PLACA_PIN					PTBD_PTBD1			// PTB2
#define		S0_MULTIPLEXOR_PERNOS_PIN				PTGD_PTGD0			// PTG0
#define		S1_MULTIPLEXOR_PERNOS_PIN				PTGD_PTGD1			// PTG1
#define		S2_MULTIPLEXOR_PERNOS_PIN				PTAD_PTAD0			// PTA0
#define		S3_MULTIPLEXOR_PERNOS_PIN				PTGD_PTGD2			// PTG2
#define		E_MULTIPLEXOR_PERNOS_PIN				PTAD_PTAD1			// PTA1

#define		S0_MULTIPLEXOR_PLACA_DIRECCION			PTDDD_PTDDD1		// PTD1
#define		S1_MULTIPLEXOR_PLACA_DIRECCION			PTDDD_PTDDD2		// PTD2
#define		S2_MULTIPLEXOR_PLACA_DIRECCION			PTBDD_PTBDD3		// PTB3
#define		S3_MULTIPLEXOR_PLACA_DIRECCION			PTDDD_PTDDD0		// PTD0
#define		E_MULTIPLEXOR_PLACA_DIRECCION			PTBDD_PTBDD2		// PTB2
#define		S0_MULTIPLEXOR_PERNOS_DIRECCION			PTGDD_PTGDD0		// PTG0
#define		S1_MULTIPLEXOR_PERNOS_DIRECCION			PTGDD_PTGDD1		// PTG1
#define		S2_MULTIPLEXOR_PERNOS_DIRECCION			PTADD_PTADD0		// PTA0
#define		S3_MULTIPLEXOR_PERNOS_DIRECCION			PTGDD_PTGDD2		// PTG2
#define		E_MULTIPLEXOR_PERNOS_DIRECCION			PTADD_PTADD1		// PTA1

#define		PERNOS_SALIDA_CONTINUIDAD_DIRECCION		PTBDD_PTBDD1		// PTB1
#define		PERNOS_SALIDA_CONTINUIDAD				AD1P1


// Defines para los canales del ADC
#define		PIN_ADC_PERNOS_CONTINUIDAD				APCTL1_ADPC1_MASK	// AD1P1
#define		PIN_ADC_LEDS							APCTL1_ADPC0_MASK	// AD1P0
#define		PIN_ADC_PELTIER							APCTL2_ADPC11_MASK	// AD1P11

#define		ADC_CANAL_PELTIER						0x0B
#define		ADC_CANAL_PERNOS						0x01
#define		ADC_CANAL_LEDS							0x00



		/****************************************************************************************************************/
		/*					  						DEFINICIONES GENERALES												*/
		/****************************************************************************************************************/

	/* Continuidad de los pernos */

#define		MINIMAS_CUENTRAS_PARA_CONTINUIDAD		100
#define		HAY_CONTINUIDAD							TRUE
#define		NO_HAY_CONTINUIDAD						FALSE
#define		TOTAL_DE_CONTACTOS_MULTIPLEXORES		16
#define		TOTAL_DE_PERNOS							7
#define		TOTAL_DE_PERNOS_MANILLAR_ENYGMA			4
#define		TOTAL_DE_CONTACTOS_MAZO_66A				14


	/* Cantidad de pernos para cada target */

// Lugar 1
#define		TOTAL_DE_PERNOS_CABLE_MAZO_001			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_041			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_105			4

// Lugar 2
#define		TOTAL_DE_PERNOS_CABLE_MAZO_002			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_042			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_072			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_106			3

// Lugar 3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_003			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_004			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_005			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_026			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_043			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_044			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_060			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_073			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_074			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_110			3

// Lugar 4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_006			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_007			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_045			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_046			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_075			3
#define		TOTAL_DE_PERNOS_CABLE_MAZO_107			4

// Lugar 5
#define		TOTAL_DE_PERNOS_CABLE_MAZO_012			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_051			4

// Lugar 6
#define		TOTAL_DE_PERNOS_CABLE_MAZO_025			2
#define		TOTAL_DE_PERNOS_CABLE_MAZO_071			2
#define		TOTAL_DE_PERNOS_CABLE_MAZO_083			2

// Lugar 7
#define		TOTAL_DE_PERNOS_CABLE_MAZO_047			5
#define		TOTAL_DE_PERNOS_CABLE_MAZO_076			5

// Lugar 8
#define		TOTAL_DE_PERNOS_CABLE_MAZO_011			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_050			4
#define		TOTAL_DE_PERNOS_CABLE_MAZO_108			4

// Lugar 9
#define		TOTAL_DE_PERNOS_CABLE_MAZO_023			2
#define		TOTAL_DE_PERNOS_CABLE_MAZO_081			2

// Lugar 10
#define		TOTAL_DE_PERNOS_CABLE_MAZO_021			2
#define		TOTAL_DE_PERNOS_CABLE_MAZO_079			2

// Lugar 11
#define		TOTAL_DE_PERNOS_CABLE_MAZO_031			2

// Lugar 12
#define		TOTAL_DE_PERNOS_CABLE_MAZO_038			4

// Lugar 13
#define		TOTAL_DE_PERNOS_CABLE_MAZO_078			2

// Lugar 14
#define		TOTAL_DE_PERNOS_CABLE_MAZO_030			6

// Lugar 15
#define		TOTAL_DE_PERNOS_CABLE_MAZO_090			2

// Lugar 16
#define		TOTAL_DE_PERNOS_CABLE_MAZO_009			8
#define		TOTAL_DE_PERNOS_CABLE_MAZO_048			8
#define		TOTAL_DE_PERNOS_CABLE_MAZO_077			8

// Lugar 17
#define		TOTAL_DE_PERNOS_CABLE_MAZO_022			2

// Lugar 18
#define		TOTAL_DE_PERNOS_CABLE_MAZO_024			9
#define		TOTAL_DE_PERNOS_CABLE_MAZO_109			7

// Lugar 19
#define		TOTAL_DE_PERNOS_CABLE_MAZO_082			7

// Lugar 20
#define		TOTAL_DE_PERNOS_CABLE_MAZO_096			7

// Lugar 21
#define		TOTAL_DE_PERNOS_CABLE_MAZO_097_A		16
#define		TOTAL_DE_PERNOS_CABLE_MAZO_097_B		7

// Lugar 23
#define		TOTAL_DE_PERNOS_CABLE_MAZO_032			14
#define		TOTAL_DE_PERNOS_CABLE_MAZO_066			14

// Lugar 24
#define		TOTAL_DE_PERNOS_CABLE_MAZO_111			7

// Lugar 25
#define		TOTAL_DE_PERNOS_CABLE_MAZO_035			4

// Lugar 26
#define		TOTAL_DE_PERNOS_CABLE_MAZO_034			7

// Lugar para el cable de telefono
#define		TOTAL_DE_PERNOS_CABLE_MAZO_000			4


	/* Teclas */

#define		MINIMO_DE_OPRESIONES_CUALQUIER_TECLA	1
#define		MINIMO_DE_VECES_SIN_REVISAR_OPRESION	6





		/****************************************************************************************************************/
		/*					  						PROTOTIPOS DE LAS FUNCIONES											*/
		/****************************************************************************************************************/


	/* FUNCIONES GENERALES */

void 			wait 						( void );
void 			EsperarOpresionDeTecla		( char pulsador );
estadosTeclas 	RevisarOpresionDeTeclas 	( void );
void 			ApagarLedVerde 				( void );
void 			ApagarLedRojo 				( void );
void 			EncenderLedVerde 			( void );
void 			EncenderLedVerde 			( void );
void 			IndicarPruebaBien 			( void );
void 			IndicarPruebaMal 			( void );
void 			ApagarLeds 					( void );
void 			AlternarLedRojo				( void );
void 			AlternarLedVerde			( void );


	/* FIRMWARE PARA LA PRUEBA REDUCIDA DEL MANILLAR */

void ColocarDireccionMultiplexorContactos	( char direccion, char target );
void ColocarDireccionMultiplexorPlaca		( char direccion, char target );
void InicializarPuertosProbador				( void );


	/* FUNCIONES PARA EL ADC */

void 	ConfigurarADC 			( void );
bool 	TomarMuestraSimpleADC	( char canal );
bool 	iniciarADC_Simple 		( uint8_t canal );
uint8_t leerADC 				( void );
void 	borrarFlagADC 			( void );


	/* FUNCIONES PARA INFORMAR LOS ERRORES */

void	InformarPernosAbiertos 		( void );
void	InformarPernosEnCorto 		( void );
void 	InformarMalUbicacion 		( void );


resultadoPernos ProbarCables ( void );

void InformarCablesAbiertos ( void );
void InformarCablesEnCorto ( void );
void InformarCablesConMalUbicacion ( void );
