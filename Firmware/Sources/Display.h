/****************************************************************************************************/
/****************************************************************************************************/
/* 								DEFINICIONES PARA MANEJAR LOS LCD 									*/
/****************************************************************************************************/
/****************************************************************************************************/

// Includes
#include "IO_Map.h"
#include "PE_Types.h"


// Las demoras de menos de 1mSeg se hacen bloqueantes
#define 	DEMORA_200US			TPM1MOD = 0x00C8;	TPM1CNT = 0;	TPM1SC |= 0x08;		while( TPM1CNT < 0x00C8 )	{};

// Las demas se realizan mediante la interrupcion del timer
#define		CUARENTA_MILI_SEGUNDOS		40000



/* Defines generales para hacer mas legible el codigo */

#define		ACTIVADO					1
#define		DESACTIVADO					0
#define		PARTE_ALTA					TRUE
#define		PARTE_BAJA					FALSE
#define 	RENGLON_SUPERIOR			0
#define 	RENGLON_MEDIO_SUPERIOR		1
#define 	RENGLON_MEDIO_INFERIOR		2
#define 	RENGLON_INFERIOR			3
#define		ROM_PARTE_ALTA				TRUE
#define		ROM_PARTE_BAJA				FALSE
#define		PARAMETRO_NO_UTILIZADO		0



/* Defines para las instrucciones de los LCD */

// Instrucciones para borrar la pantalla
#define 	INSTRUCCION_LCD_BORRAR_PANTALLA		0x01

// Instrucciones para regresar el cursor al inicio
#define 	INSTRUCCION_LCD_REGRESAR_CURSOR		0x20

// Instrucciones para seleccionar el modo
#define 	INSTRUCCION_LCD_MODO				0x04
#define 	INSTRUCCION_LCD_MODO_INCREMENTO		0X02
#define 	INSTRUCCION_LCD_MODO_DECREMENTO		0X00
#define 	INSTRUCCION_LCD_MODO_SHIFT_ON		0X01
#define 	INSTRUCCION_LCD_MODO_SHIFT_OFF		0X00

// Instrucciones para encender o apagar la pantalla
#define 	INSTRUCCION_LCD_PANTALLA			0x08
#define 	INSTRUCCION_LCD_PANTALLA_ON			0x04
#define 	INSTRUCCION_LCD_PANTALLA_OFF		0x00
#define 	INSTRUCCION_LCD_PANTALLA_CURSOR_ON	0x02
#define 	INSTRUCCION_LCD_PANTALLA_CURSOR_OFF	0x00
#define 	INSTRUCCION_LCD_PANTALLA_BLINK_ON	0x01
#define 	INSTRUCCION_LCD_PANTALLA_BLINK_OFF	0x00

// Instrucciones para desplazar el cursor o la pantalla
#define 	INSTRUCCION_LCD_DESPLAZAR			0x10
#define 	INSTRUCCION_LCD_DESPLAZAR_PANTALLA	0x08
#define 	INSTRUCCION_LCD_DESPLAZAR_CURSOR	0x00
#define 	INSTRUCCION_LCD_DESPLAZAR_DERECHA	0x04
#define 	INSTRUCCION_LCD_DESPLAZAR_IZQUIERDA	0x00

// Instrucciones para definir la interfaz
#define 	INSTRUCCION_LCD_INTERFAZ			0x20
#define 	INSTRUCCION_LCD_INTERFAZ_8BITS		0x10
#define 	INSTRUCCION_LCD_INTERFAZ_4BITS		0x00
#define 	INSTRUCCION_LCD_INTERFAZ_2LINEAS	0x08
#define 	INSTRUCCION_LCD_INTERFAZ_1LINEA		0x00
#define 	INSTRUCCION_LCD_INTERFAZ_5X10		0x04
#define 	INSTRUCCION_LCD_INTERFAZ_5X8		0x00

// Instrucciones para definir el renglon sobre el que se escribe
#define 	INSTRUCCION_LCD_RENGLON_SUPERIOR			0x80
#define 	INSTRUCCION_LCD_RENGLON_MEDIO_SUPERIOR		INSTRUCCION_LCD_RENGLON_SUPERIOR + 0x40
#define 	INSTRUCCION_LCD_RENGLON_MEDIO_INFERIOR		INSTRUCCION_LCD_RENGLON_SUPERIOR + 20
#define 	INSTRUCCION_LCD_RENGLON_INFERIOR			INSTRUCCION_LCD_RENGLON_MEDIO_SUPERIOR + 20


// Mascaras para acceder a los bits de datos
#define		MASCARA_DB7_DATO_PROBADOR		0x80		// Para quedarse con x0000000
#define		MASCARA_DB6_DATO_PROBADOR		0x40		// Para quedarse con 0x000000
#define		MASCARA_DB5_DATO_PROBADOR		0x20		// Para quedarse con 00x00000
#define		MASCARA_DB4_DATO_PROBADOR		0x10		// Para quedarse con 000x0000
#define		MASCARA_DB3_DATO_PROBADOR		0x08		// Para quedarse con 0000x000
#define		MASCARA_DB2_DATO_PROBADOR		0x04		// Para quedarse con 00000x00
#define		MASCARA_DB1_DATO_PROBADOR		0x02		// Para quedarse con 000000x0
#define		MASCARA_DB0_DATO_PROBADOR		0x01		// Para quedarse con 0000000x



// Data direction - Para elegir si son entradas o salidas
#define  	LCD_RS_DD							PTCDD_PTCDD4 	//rs
#define 	LCD_E_DD							PTGDD_PTGDD3 	//e
#define 	LCD_DB4_DD 							PTCDD_PTCDD1 	//db4
#define 	LCD_DB5_DD  						PTCDD_PTCDD0 	//db5
#define		LCD_DB6_DD							PTGDD_PTGDD6 	//db6
#define 	LCD_DB7_DD							PTGDD_PTGDD5 	//db7

// Data - Para escribir o leer los bits de datos
#define  	LCD_RS								PTCD_PTCD4		//rs
#define 	LCD_E								PTGD_PTGD3		//e
#define 	LCD_DB4 							PTCD_PTCD1		//db4
#define 	LCD_DB5	    						PTCD_PTCD0		//db5
#define		LCD_DB6								PTGD_PTGD6		//db6
#define 	LCD_DB7								PTGD_PTGD5		//db7


// Defines para seleccionar que tipo de LCD se va a usar
#define		LCD_20_CARACTERES		20
#define		LCD_16_CARACTERES		16
#define		LCD_8_CARACTERES		8
#define		LCD_2_FILAS				2
#define		LCD_4_FILAS				4
#define		LCD_4_BITS				TRUE
#define		LCD_8_BITS				FALSE
#define		LCD_PUERTO_EXTERNO		TRUE
#define		LCD_PUERTO_INTERNO		FALSE
#define		LCD_FUENTE_CHICA		TRUE
#define		LCD_FUENTE_GRANDE		FALSE
#define		LCD_MANGO_VACIO			FALSE
#define		LCD_PROBADOR			TRUE


// Definicion de una estructura para manejar mas facilmente al LCD, como si fuesen un objeto
typedef struct
{
	char	filas;
	char	caracteres;
	bool	fuente;
	bool	interfaz;
	bool	puerto;
	bool	probador;
}LCD_Objeto;

// Mensajes predefinidos
typedef enum mensajesLCD { INICIADO = 0, SIN_SENSORES, BUSCANDO, SENSORES_ENCONTRADOS, TEXTO_FUNCION };







/* Prototipos de las funciones */

// De uso externo
bool SeleccionarLCD( char totalDeFilas, char totalDeCaracteres, bool fuente, bool interfaz_4bits, bool puertoExterno, bool probador );
void InicializarLCD( void );
void MostrarMensajeLCD( char tipoDeMensaje, char parametroExtra );
bool EscribirMensajeLCD( unsigned char renglon, unsigned char inicioTexto, unsigned char totalDeCaracteres, unsigned char *pTexto);
void EscribirTemperatura( int16_t temperatura, char reglon, char offset );
void EscribirVariacionesDeTemperatura( int16_t variacionDeTemperatura, char reglon, char offset );
void EscribirCorriente( int corriente_mA, char reglon, char offset );
void EscribirROM_EnLCD( unsigned char *ROM );
void BorrarLCD( void );

// De uso interno
void enviarInstruccion( char dato );
void enviarDato( char dato );
void mandarNibbleAltoLCD_Vacio( char dato );
void mandarNibbleBajoLCD_Vacio( char dato );
void mandarNibbleAltoLCD_Probador( char dato );
void mandarNibbleBajoLCD_Probador( char dato );
void inicializarLCD_4bits( void );
void inicializarPuertoLCD( void );

void escribirMediaROM_EnLCD( unsigned char renglon, unsigned char *ROM, bool parteAlta );
void escribirTodaROM_EnLCD( unsigned char renglon, unsigned char *ROM );


void demoraInicialLCD( void );


void mensajeLCD_Iniciado ( void );
void mensajeLCD_SinSensores( void );
void mensajeLCD_BuscandoSensores( void );
void mensajeLCD_FuncionDelSensor ( char posicion );
void mensajeLCD_SensoresEncontrados ( char encontrados );

void borrarRenglon( char renglon );


void ImprimirMensajeErrorPruebaReducida ( char sensor );
void MostrarTemperaturasPruebaReducida ( int16_t sensorSoldado, int16_t sensorLadoCaliente );


void EscribirUbicacion( char ubicacion, char reglon, char offset );
void EscribirCifra( char cifra, char renglon, char offset );
void EscribirNumeroDeCable( char cable, char reglon, char offset );
