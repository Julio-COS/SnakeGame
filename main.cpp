#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#include <windows.h>
#include <mmsystem.h>

#include <GL/gl.h>
#include <ctime>
#include <iostream>

//#define FPS 5
#define UP 1
#define DOWN -1
#define RIGHT 2
#define LEFT -2

float DIRECCION=RIGHT;
float limite[]={600,-600};
int tamanioSerpi=5;
float posX[60]={0,0,0,0,0}; 
float posY[60]={60,0,-60,-120,-180};
float rgb[]={0.0,0.0,0.0};
float comidaX, comidaY;
bool comida=true;
int puntuacion = 0;
int velocidadGusano = 5; //velocidad inicio del gusano


int texturaActual = 0; // 0 para "menu" y 1 para "niveles"
int levelOne = false;
// Prototipo de la función playButtonClick
void playButtonClick();

// Prototipos para funciones de niveles
void levelOneButtonClick();
void levelTwoButtonClick();

// Estructura para representar un botón general
struct Button {
    int x, y, width, height;
    void (*onClick)();
};

// Estructura para representar botón de menú principal
Button playButton = {210, 420, 250, 50, playButtonClick};

// Estructura para representar botón de menú por niveles
Button levelOneButton = {170, 300, 250, 50, levelOneButtonClick};
Button levelTwoButton = {170, 380, 250, 50, levelTwoButtonClick};

typedef struct{
	GLubyte *dibujo;
	GLuint bpp;
	GLuint largo;
	GLuint ancho;
	GLuint ID;
}textura;

textura menu;
textura levels;
textura fondo;
textura fondo2;
textura piel;
textura ojo;

int cargaTGA(char const*nombres, textura *imagen){
	GLubyte cabeceraTGA[12]={0,0,2,0,0,0,0,0,0,0,0,0};
	GLubyte compararTGA[12];//0,0,2,0,0,0,0,0,0,0,0,0
	GLubyte cabecera[6];//11,5,11,12,32,40
	GLuint bytesporpuntos;
	GLuint tamanoimagen;
	GLuint temp,i;
	GLuint tipo=GL_RGBA;
	
	FILE *archivo=fopen(nombres,"rb");
	if(
	archivo==NULL ||
	fread(compararTGA,1,sizeof(compararTGA),archivo)!= sizeof(compararTGA) ||
	memcmp(cabeceraTGA,compararTGA,sizeof(compararTGA))!=0 ||
	fread(cabecera,1,sizeof(cabecera),archivo)!= sizeof(cabecera)
	){
	 if(archivo==NULL){
	 	printf("No se encontro el archivo %s\n", nombres);
	 	return 0;
	 }else{
	 	fclose(archivo);
	 	return 0;
	 }	
		
	}
	imagen->largo=256*cabecera[1] +cabecera[0];//256*5  + 11= 256+244= 500
    imagen->ancho=256*cabecera[3] +cabecera[2];//256*1  + 244= 256+244= 500

     if(
	 imagen->largo <=0 ||
	 imagen->ancho <=0 ||
	 (cabecera[4]!=24 && cabecera[4]!=32)
	 ){
	 	printf("Datos invalidos\n");
	 	fclose(archivo);
	 	return 0;
	 }
     imagen->bpp=cabecera[4];
     bytesporpuntos=cabecera[4]/8;
     tamanoimagen=imagen->largo * imagen->ancho *bytesporpuntos;
      
    imagen->dibujo=(GLubyte *)malloc(tamanoimagen);
    if(
	imagen->dibujo==NULL ||
	fread(imagen->dibujo,1,tamanoimagen,archivo)!=tamanoimagen
	){
	   if(imagen->dibujo!=NULL){
	   	printf("Error leyendo imagen\n");
	   	free(imagen->dibujo);
	   }else {
	   	printf("Error asignando memoria\n");
	   }
	   
	   fclose(archivo);
	   return 0;	
	}
	
    for(i=0;i<(int)tamanoimagen;i+=bytesporpuntos){
    	temp=imagen->dibujo[i];
    	imagen->dibujo[i]=imagen->dibujo[i+2];
    	imagen->dibujo[i+2]=temp;
	}
    fclose(archivo);
    
    glGenTextures(1,&imagen[0].ID);
    glBindTexture(GL_TEXTURE_2D,imagen[0].ID);
    if(imagen->bpp==24) tipo=GL_RGB;
    
    glTexImage2D(GL_TEXTURE_2D,0,tipo,imagen[0].ancho,imagen[0].largo,0,tipo,GL_UNSIGNED_BYTE,imagen[0].dibujo);
    
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    return 1;
    
}

void init(){
	//CARGA DE TEXTURAS
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0);//borrando cualquier tipo de profundidad
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	//carga de textura
	if(!cargaTGA("menu_600x600.tga",&menu)){
		printf("Error cargando textura\n");
		exit(0);
	}	
	if(!cargaTGA("niveles_600x600.tga",&levels)){
		printf("Error cargando textura\n");
		exit(0);
	}
	if(!cargaTGA("textura03.tga",&fondo)){
		printf("Error cargando textura\n");
		exit(0);
	}
	if(!cargaTGA("textura02.tga",&fondo2)){
		printf("Error cargando textura\n");
		exit(0);
	}
	if(!cargaTGA("piel.tga",&piel)){
		printf("Error cargando textura\n");
		exit(0);
	}
	if(!cargaTGA("ojo.tga",&ojo)){
		printf("Error cargando textura\n");
		exit(0);
	}
	//CARGA DE SERPIENTE
	//luz
	GLfloat light_position[]={1,-1,1,0};
	glEnable(GL_BLEND);//combinar valores de los colores
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_POSITION,light_position);
	
	//profundidad
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);//Compara profundidad
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}

void handleMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    	if(texturaActual == 0) {
        	// Verifica si el clic del mouse está dentro de las coordenadas del botón
	        if (x >= playButton.x && x <= playButton.x + playButton.width &&
	            y >= playButton.y && y <= playButton.y + playButton.height) {
	            //llama variable asignada con evento click()
	            playButton.onClick();
	        }
	    } else {
	    	if (x >= levelOneButton.x && x <= levelOneButton.x + levelOneButton.width &&
	            y >= levelOneButton.y && y <= levelOneButton.y + levelOneButton.height) {
	            //llama variable asignada con evento click()	
	            levelOneButton.onClick();
	        } else if (x >= levelTwoButton.x && x <= levelTwoButton.x + levelTwoButton.width &&
	            	   y >= levelTwoButton.y && y <= levelTwoButton.y + levelTwoButton.height){
	        	//llama variable asignada con evento click()	
	            levelTwoButton.onClick();
			}
		}
    } 
}

// Función para dibujar un botón
void drawButton(const Button& button, const char* label) {
    glColor3f(0.5, 0.5, 0.5); // Color del botón
    glBegin(GL_QUADS);
    glVertex2f(button.x, button.y);
    glVertex2f(button.x + button.width, button.y);
    glVertex2f(button.x + button.width, button.y + button.height);
    glVertex2f(button.x, button.y + button.height);
    glEnd();

    glColor3f(1.0, 1.0, 1.0); // Color del texto
    glRasterPos2f(button.x + button.width / 2 - 25, button.y + button.height / 2);

    // Cambié la función a glutBitmapCharacter
    for (const char* c = label; *c; ++c) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void playButtonClick() {
    texturaActual = 1;
    printf("¡Botón de Jugar clickeado!\n");
}

void levelOneButtonClick() {
    //llama nivel uno
    levelOne = true;
    texturaActual = 3;
    printf("¡Botón de nivel 1 clickeado!\n");
}

void levelTwoButtonClick() {
    //llama nivel dos
    levelOne = false;
    texturaActual = 3;
    printf("¡Botón de nivel 2 clickeado!\n");
}

void luz(){
	GLfloat mat_ambient[]={0.0f,1.0f,0.0f,1.0f};
	GLfloat mat_diffuse[]={0.0f,0.5f,0.5f,1.0f};
	GLfloat mat_specular[]={1.0f,1.0f,1.0f,1.0f};
	GLfloat mat_shininess[]={122};
	
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	
}

void dibujarTabla(textura text){
	glBindTexture(GL_TEXTURE_2D, text.ID);
    glBegin(GL_QUADS);
   
    glTexCoord2f(0.0, 0.0); glVertex3d(limite[1], limite[0], 0);
    glTexCoord2f(1.0, 0.0); glVertex3d(limite[0], limite[0], 0);
    glTexCoord2f(1.0, 1.0); glVertex3d(limite[0], limite[1], 0);
    glTexCoord2f(0.0, 1.0); glVertex3d(limite[1], limite[1], 0);
    glEnd();
}

bool gameOver(){
	//muro
	if(posX[0]>limite[0]-1 ||  posY[0]>limite[0]-1 ||
	   posX[0]<limite[1] ||  posY[0]<limite[1] ){
			return true;	
		}

	//cuerpo
	for(int i = tamanioSerpi-1;i>0;i--){
		if(posX[0]==posX[i] && posY[0]==posY[i]){
			return true;
		}
	}
	return false;
}

void moverSerpiente(){
	
	glEnable(GL_TEXTURE_2D);

	glDisable(GL_LIGHTING);
	GLUquadric*qobj=gluNewQuadric();
	gluQuadricTexture(qobj,GL_TRUE);
	glBindTexture(GL_TEXTURE_2D,piel.ID);
	glEnable(GL_LIGHTING);
	
	
	int velMovimiento=60;
	
	//le sigue el cuerpo
	for(int i = tamanioSerpi-1;i>0;i--){
		posX[i]=posX[i-1];
		posY[i]=posY[i-1];
	}
	
	if(DIRECCION==UP){
		posY[0]+=velMovimiento;
	}
	else if(DIRECCION==DOWN){
		posY[0]-=velMovimiento;
	}	
	else if(DIRECCION==RIGHT){
		posX[0]+=velMovimiento;
	}
	else if(DIRECCION==LEFT){
		posX[0]-=velMovimiento;
	}

	// Dibuja el cuerpo de la serpiente con esferas
    for (int i = 1; i < tamanioSerpi; i++) {
    	
        glLoadIdentity();
        glTranslated(posX[i] + 30, posY[i] + 30, 0);
        gluSphere(qobj,15, 20, 20);
    }
	
	//Código para rotar la cabeza de la serpiente
	glLoadIdentity();
	glTranslated(posX[0]+30,posY[0]+30,0);
	
	if(DIRECCION==UP){
		glRotated(270,1,0,0);
	}
	else if(DIRECCION==DOWN){
		glRotated(90,1,0,0);
	}	
	else if(DIRECCION==RIGHT){
		glRotated(90,0,1,0);
	}
	else if(DIRECCION==LEFT){
		glRotated(270,0,1,0);
	}
	glBindTexture(GL_TEXTURE_2D,piel.ID);
	//BOCA
	glutSolidCone(22,40,100,100);
	//CABEZA
	gluSphere(qobj,20,20,20);
	
	//Creacion de los ojos
	glBindTexture(GL_TEXTURE_2D,ojo.ID);
	
	glDisable(GL_LIGHTING);
	if(DIRECCION==UP){
		glTranslated(-15,0,5);
		gluSphere(qobj,10,50,50);
		
		glTranslated(30,0,0);
		gluSphere(qobj,10,50,50);

	}
	else if(DIRECCION==DOWN){
		glRotated(190,0,0,1);
		glTranslated(-15,0,0);
		gluSphere(qobj,10,50,50);
		
		glTranslated(30,0,0);
		gluSphere(qobj,10,50,50);
	}	
	else if(DIRECCION==RIGHT){
		glTranslated(-15,0,0);
	}
	else if(DIRECCION==LEFT){
		glTranslated(15,0,0);
	}
	//OJO 01
	glTranslated(0,16,0);
	gluSphere(qobj,10,50,50);
	//OJO 02
	glTranslated(0,-32,0);
	gluSphere(qobj,10,50,50);
	
	
	//evaluar comida
	if((posX[0] == comidaX && posY[0] == comidaY)) //||
	{
		tamanioSerpi++;
		comida=true;
		puntuacion++;
		
		// Cada vez que el jugador obtenga 10 puntos adicionales, aumenta la velocidad del gusano
    	if (puntuacion % 10 == 0 && puntuacion != 0) {
        	velocidadGusano += 2; // Aumenta la velocidad en 2
    	}
	}

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
}

void rng(float &x, float &y){
	int maxX = 10;
	int maxY = 10;
	int minimo = -10;
	srand(time(NULL));
	x= (minimo + rand() % (maxX-minimo))*60;
	y= (minimo + rand() % (maxY-minimo))*60;
	rand();
}

void dibujarComida(){
	if(comida){
		rng(comidaX,comidaY);
	}
	comida=false;
	
	//Se traslada a la posicion
	glLoadIdentity();
	glTranslated(comidaX+30,comidaY+30,0);
	glutSolidSphere(15,20,20);
}

void timer(int){
	glutPostRedisplay();
	glutTimerFunc(1000/velocidadGusano,timer,0);
}

void keyboard(int key, int, int){
	switch(key){
		case GLUT_KEY_UP:
			if(DIRECCION!=DOWN)
				DIRECCION=UP;
			break;
		case GLUT_KEY_DOWN:
			if(DIRECCION!=UP)
				DIRECCION=DOWN;
			break;
		case GLUT_KEY_RIGHT:
			if(DIRECCION!=LEFT)
				DIRECCION=RIGHT;
			break;
		case GLUT_KEY_LEFT:
			if(DIRECCION!=RIGHT)
				DIRECCION=LEFT;
			break;
		
	}
}

void reiniciarJuego() {
   DIRECCION = RIGHT;
    tamanioSerpi = 5;
    for (int i = 0; i < 60; i++) {
        posX[i] = 0;
        posY[i] = 60 * i;
    }
    comida = true;
    puntuacion = 0;
    velocidadGusano = 5;
   
    // Vuelve a cargar la textura del fondo según el nivel
    if (levelOne) {
        cargaTGA("textura03.tga", &fondo);
    } else {
        cargaTGA("textura02.tga", &fondo2);
    }
}
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW_MATRIX);
	glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.2);
	glDisable(GL_LIGHTING);
	glColor3f(1.0f,1.0f,1.0f);
	int d = 600;
	
    if (texturaActual == 0) {
        glBindTexture(GL_TEXTURE_2D, menu.ID);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3d(-d, d, 0);
        glTexCoord2f(1.0, 0.0);
        glVertex3d(d, d, 0);
        glTexCoord2f(1.0, 1.0);
        glVertex3d(d, -d, 0);
        glTexCoord2f(0.0, 1.0);
        glVertex3d(-d, -d, 0);
        glEnd();
    } else if (texturaActual == 1) {
        glBindTexture(GL_TEXTURE_2D, levels.ID);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3d(-d, d, 0);
        glTexCoord2f(1.0, 0.0);
        glVertex3d(d, d, 0);
        glTexCoord2f(1.0, 1.0);
        glVertex3d(d, -d, 0);
        glTexCoord2f(0.0, 1.0);
        glVertex3d(-d, -d, 0);
        glEnd();
    } else {
    	if(levelOne == true) {
    		//CASO NIVEL 1
		    glLoadIdentity();	    
			dibujarTabla(fondo);
			glLoadIdentity();
			glEnable(GL_LIGHTING);
				luz();
				moverSerpiente();
				glLoadIdentity();
				dibujarComida();
			glDisable(GL_LIGHTING);
			
			if (gameOver()) {
		    	char puntuacionStr[50];
			    sprintf(puntuacionStr, "Puntuacion: %d", puntuacion);
			    MessageBox(NULL, puntuacionStr, "GAME OVER", 0);
			    reiniciarJuego();
			    texturaActual = 0;
			}
		} else {
			//CASO NIVEL 2
		    glLoadIdentity();		    
			dibujarTabla(fondo2);
			glLoadIdentity();
			glEnable(GL_LIGHTING);
				luz();
				moverSerpiente();
				glLoadIdentity();
				dibujarComida();
			glDisable(GL_LIGHTING);
			
			if (gameOver()) {
		    	char puntuacionStr[50];
			    sprintf(puntuacionStr, "Puntuacion: %d", puntuacion);
			    MessageBox(NULL, puntuacionStr, "GAME OVER", 0);
			    reiniciarJuego();
			    texturaActual = 0;
			}
		}
	}	
	
	glutSwapBuffers();
	
}

void reshape(int largo, int ancho){
	glViewport(0,0,(GLsizei)largo,(GLsizei)ancho);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	int t=600;
	//600
	glOrtho(-t,t,-t,t,-t,t);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void playAudio(const char* audioFilePath) {
    // Reproduce un archivo de audio utilizando WinMM
    PlaySound(audioFilePath, NULL, SND_ASYNC);
}

int main(int argc, char** argv) {
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA |GLUT_DOUBLE |GLUT_DEPTH | GLUT_SINGLE);
	glutInitWindowSize(600,600);
	glutInitWindowPosition(20,20);
	glutCreateWindow("Proyecto Serpiente");
	init();
	glutMouseFunc(handleMouseClick);
	playAudio("Audio.wav");
	glutDisplayFunc(display);

	glutReshapeFunc(reshape);
	glutTimerFunc(0,timer,0);
	glutSpecialFunc(keyboard);
	glutMainLoop();
	
	return 0;
}
