#include "stm32f4xx_hal.h"
#include <math.h>
#include "calc_sinusM.h"
//constantes utiles pour les calculs numériques
const float un_s_racine3 = 1.0/sqrt(3.0);
const float DPI_S3 = 2.0*M_PI/3.0;
const float QPI_S3 = 4.0*M_PI/3.0;

//variables nécessaires pour les relations trigonométriques qui permettent la transformation du signal des sondes
//de hall en sinus M
float sin_beta = 0;
float cos_beta = 0;
float sin_beta120 = 0;
float cos_beta120 = 0;
float sin_beta240 = 0;
float cos_beta240 = 0;

/*gestion des min max des sondes de hall*/

volatile uint16_t minhall0 =65535;
volatile uint16_t maxhall0=0;
volatile uint16_t minhall1 =65535;
volatile uint16_t maxhall1=0;
volatile uint16_t minhall2 =65535;
volatile uint16_t maxhall2=0;

//gestion des offsets et des facteurs pour mettre les signaux Hall à l'echelle entre -1 et 1
float fact0=0.000968054f;
float offset0=2240.0;
float fact1=0.000968054f;
float offset1=2240.0;
float fact2=0.000968054f;
float offset2=2240.0;

//mise à l'échelle des sondes de hall de 0 à 4095 -> -1 à 1
//cette fonction est à appeler dans l'interruption
void hall_scale(uint16_t* measured_halls, float* scaled_halls )
{
/*gestion des min max des sondes de hall*/
	if (measured_halls[0]<minhall0)minhall0=measured_halls[0];
	if (measured_halls[0]>maxhall0)maxhall0=measured_halls[0];
	if (measured_halls[1]<minhall1)minhall1=measured_halls[1];
	if (measured_halls[1]>maxhall1)maxhall1=measured_halls[1];
	if (measured_halls[2]<minhall2)minhall2=measured_halls[2];
	if (measured_halls[2]>maxhall2)maxhall2=measured_halls[2];
	/*fin de la gestion des min max des sondes de hall*/
	scaled_halls[0] = (measured_halls[0]-offset0)*fact0;
	scaled_halls[1] = (measured_halls[1]-offset1)*fact1;
	scaled_halls[2] = (measured_halls[2]-offset2)*fact2;
	return;
}

//recalcul des min-max et des offsets
//comme il y a une division => à appeler dans la boucle infinie de la fonction main
void calc_hallminmax(void)
{
	uint16_t offsettmp;
	/*gestion des min max des sondes de hall*/
	offsettmp=(maxhall0+minhall0)>>1;
	if((offsettmp>1500)&&(offsettmp<2500))offset0=(float)offsettmp;
	if (((maxhall0-minhall0)<2300)&&((maxhall0-minhall0)>1500))fact0=2.0/(maxhall0-minhall0);

	offsettmp=(maxhall1+minhall1)>>1;
	if((offsettmp>1500)&&(offsettmp<2500))offset1=(float)offsettmp;
	if (((maxhall1-minhall1)<2300)&&((maxhall1-minhall1)>1500))fact1=2.0/(maxhall1-minhall1);

	offsettmp=(maxhall2+minhall2)>>1;
	if((offsettmp>1500)&&(offsettmp<2500))offset2=(float)offsettmp;
	if (((maxhall2-minhall2)<2300)&&((maxhall2-minhall2)>1500))fact2=2.0/(maxhall2-minhall2);
	/*fin de la gestion des min max des sondes de hall*/
}

//calcul des sinus liés au déphasage (à appeler dans le main)
void calc_betas(float beta)
{
	sin_beta = sin(beta);
	sin_beta120 = sin(beta-DPI_S3);
	sin_beta240 = sin(beta-QPI_S3);
	cos_beta = cos(beta);
	cos_beta120 = cos(beta-DPI_S3);
	cos_beta240 = cos(beta-QPI_S3);
}

//fonction qui traduit les sondes de hall (entre -1 et 1) en des signaux sinus M (entre 0 et 2)
void calc_sinusM(float* halls, float* sinusM)
{
	//on traduit les signaux entre 0 et 1 en des signaux entre -1 et 1 pour faire les calculs trigo
	float cos_alpha = halls[0];
	float sin_alpha = (halls[1]-halls[2])*0.577350269f;
	float s0 = sin_alpha*cos_beta+cos_alpha*sin_beta;
	float s1 = sin_alpha*cos_beta120+cos_alpha*sin_beta120;
	float s2 = sin_alpha*cos_beta240+cos_alpha*sin_beta240;
	float smallest = s0;
	if (s1<smallest ) smallest = s1;
	if (s2<smallest ) smallest = s2;
	sinusM[0] = s0-smallest;
	sinusM[1] = s1-smallest;
	sinusM[2] = s2-smallest;
}

//regroupe les deux fonction à appeler dans le main
//le paramètre decalage est l'angle de déphasage entre sondes de hall et signal sinus en radian
void calc_params_sinusM(float decalage)
{
calc_betas(decalage);
calc_hallminmax();
}
