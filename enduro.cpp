#include <stdio.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>

#include <unistd.h>
// Quando compilar em ubuntu trocar o Sleep por usleep(sleepTime);

//Declaração de Variáveis Globais
enum CameraType{
	rear,
	top,
	left
};

int cont = 0;

//Camera
CameraType camera;
int posx = 0, posy = 3, posz = 10; //Variáveis que definem a posição da câmera 0,3,10
int oy = 0, ox = 0, oz = -10;	//Variáveis que definem para onde a câmera olha 0,0,-10
int lx = 0, ly = 1, lz = 0;	//Variáveis que definem qual eixo estará na vertical do monitor (cima).

//Carro
float km = 0;	//Distancia percorrida
float ang = 0;	//Angulo de rotação da pista
float vel = 0.3; //Velocidade do carro
float vel_ang; //Velocidade na curva - Calculada baseado no raio e velocidade
float pos_car = 0; //Posicao do carro (somente no eixo X)
float pos_car_final = 0;
float vel_x = 0.0025; // Velocidade no eixo X (para mover para os lados)
float car_rot = 0;
float mov = 0.5; //Tamanho do movimento do carro (Esquerda ou direita)
int curva = 0; // Determina se está se movendo em curva ou não

//Pista
float largura = 8; //Largura da Pista
float comprimento = 80; //Comprimento da Pista 78
float raio_curva; //Raio da curva - Calculado com base no comprimento da pista
int atual = 0; //Pista atual 
int prox = 0; //Proxima pista
float fim_pista1x = 0, fim_pista1z = 0, fim_pista2x = 0, fim_pista2z = 0; //vertices que marcam o fim da pista
float numquads = 12; //Numero de quadrados que forma a curva
float cos_ang; //Variável para calculo de vertices da curva
float sen_ang; //Idem anterior

//Cores
float tempo = 0, relogio = 0.0005; //Tempo de um período(de 0 a 1) e o incremento no tempo
/**DESATIVADO**/
//int periodo = 0, espera = 0; //Indica qual perido está (0-Dia; 1-Tarde; 2-Noite) e flag para iniciar troca de periodo (0 indica troca)
/*DESATIVADO*/
int cor=3; //Cor do carro
int cor_i=5; //Cor dos inimigos
int r_pista = 55, g_pista = 43, b_pista = 69; //Cor inicial da pista
int r_chao = 25, g_chao = 13, b_chao = 39; //Cor inicial do chão
float r_ceu = 0, g_ceu = 0, b_ceu = 0; //Cor inicial do céu
float r_ceu_temp, g_ceu_temp, b_ceu_temp;

//Inimigos
float dist = 40; //Distância entre inimigos (em unidades de velocidade)
float vel_inimigos;
#define max_inimigos 5 //Número máximo de inimigos de uma vez
float inimigos[max_inimigos][2]; //Tabela com a distância do inimigo do jogador e onde na pista está.
int num_inimigos = 0; //Número atual de inimigos
float dist_jogador; //Distância do Jogador quando passa no mesmo Z que ele
int prct=15; //Serve para calcular em porcentagem onde na pista, existem prct+1 posições em x possíveis para os inimigos estarem

//Varíaveis para controle de estado do jogo
float vel_p;
float vel_ang_p;
float vel_inimigos_p;
float mov_p;
float relogio_p;
float pn = 5, pn_cont=0, pn2=-5;

// Lock framerate
long lastFrameTime = 0;


/****** CÓDIGO ******/

void Carro();

void Inimigo();

void Skyline0();
void Skyline1();
void Skyline2();

void drawBitmapText(char *string, float x, float y, float z){
	char *c;
	glRasterPos3f(x, y, z);
	for (c = string; *c != '\0'; c++){
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
			//glutSwapBuffers();
	}
}

void Pause() {
	vel = 0;
	vel_ang = 0;
	vel_inimigos = 0;
	mov = 0;
	relogio = 0;
}

void Resume() {
	vel = vel_p;
	vel_ang = vel_ang_p;
	vel_inimigos = vel_inimigos_p;
	mov = mov_p;
	relogio = relogio_p;
}

void Colisao() {
	dist_jogador = pos_car_final - (inimigos[0][1] - largura / 2);
	//printf("%f\n", dist_jogador);
	if (dist_jogador >= -1.1 && dist_jogador <= 1.1) {
		Pause();
		num_inimigos--;
	}
}

void Spawn() {
	
	int i, j;
	float aux;
	if (num_inimigos<max_inimigos) {
		if (num_inimigos == 0) {
			aux = rand() % (prct + 1);
			aux = aux / prct;
			inimigos[0][1] = 0.55+(aux*(largura-1));
			inimigos[0][0] = dist;
			num_inimigos++;
			for (i = num_inimigos; i<max_inimigos; i++) {
				aux = rand() % (prct + 1);
				aux = aux / prct;
				inimigos[i][1] = 0.55+(aux*(largura-1));
				inimigos[i][0] = inimigos[i - 1][0] + dist;
				num_inimigos++;
			}
		}
		else {
			for (i = 0; i<num_inimigos; i++) {
				inimigos[i][0] = inimigos[i + 1][0];
				inimigos[i][1] = inimigos[i + 1][1];
			}
			aux = rand() % (prct + 1);
			aux = aux / prct;
			inimigos[max_inimigos - 1][1] = 0.55+(aux*(largura-1));
			inimigos[max_inimigos - 1][0] = inimigos[max_inimigos - 2][0] + dist;
			num_inimigos++;
		}
	}

	for (i = 0; i<max_inimigos; i++) {
		if (atual == 0) {
			if (inimigos[i][0] <= comprimento - km) {
				inimigos[i][0] = inimigos[i][0] - vel_inimigos;
				glPushMatrix();
				glTranslatef(inimigos[i][1] - (largura / 2),0, -inimigos[i][0]);
				glRotatef(180,0,1,0);
				Inimigo();
				glPopMatrix();
			}
			else {
				if (prox == 0) {
					inimigos[i][0] = inimigos[i][0] - vel_inimigos;
					glPushMatrix();
					glTranslatef(inimigos[i][1] - (largura / 2), 0, -inimigos[i][0]);
					glRotatef(180,0,1,0);
					Inimigo();
					glPopMatrix();
				}
				else if (prox == 1) {
					inimigos[i][0] = inimigos[i][0] - vel_inimigos;
					aux = inimigos[i][0] - (comprimento - km);
					glPushMatrix();
					glTranslatef(-raio_curva - (largura / 2), 0, km - comprimento);
					glRotatef(vel_ang*aux / vel, 0, 1, 0);
					glTranslatef(raio_curva + inimigos[i][1], 0, 0);
					glRotatef(180,0,1,0);
					Inimigo();
					glPopMatrix();
				}
				else if (prox == 2) {
					inimigos[i][0] = inimigos[i][0] - vel_inimigos;
					aux = inimigos[i][0] - (comprimento - km);
					glPushMatrix();
					glTranslatef(raio_curva + (largura / 2), 0, km - comprimento);
					glRotatef(-vel_ang*aux / vel, 0, 1, 0);
					glTranslatef(-raio_curva - largura + inimigos[i][1], 0, 0);
					glRotatef(180,0,1,0);
					Inimigo();
					glPopMatrix();
				}
			}
		}
		else if (atual == 1) {
			if (inimigos[i][0] <= comprimento - km) {
				inimigos[i][0] = inimigos[i][0] - vel_inimigos;
				glPushMatrix();
				glTranslatef(-raio_curva - (largura / 2), 0, 0);
				glRotatef(vel_ang*inimigos[i][0] / vel, 0, 1, 0);
				glTranslatef(raio_curva + inimigos[i][1], 0, 0);
				glRotatef(180,0,1,0);
				Inimigo();
				glPopMatrix();
			}
			else {
				if (prox == 0) {
					inimigos[i][0] = inimigos[i][0] - vel_inimigos;
					aux = inimigos[i][0] - (comprimento - km);
					glPushMatrix();
					glTranslatef(-raio_curva - (largura / 2), 0, 0);
					glRotatef(90 - ang, 0, 1, 0);
					glTranslatef(raio_curva + inimigos[i][1], 0, -aux);
					glRotatef(180,0,1,0);
					Inimigo();
					glPopMatrix();
				}
				else if (prox == 1) {
					inimigos[i][0] = inimigos[i][0] - vel_inimigos;
					glPushMatrix();
					glTranslatef(-raio_curva - (largura / 2), 0, 0);
					glRotatef(vel_ang*inimigos[i][0] / vel, 0, 1, 0);
					glTranslatef(raio_curva + inimigos[i][1], 0, 0);
					glRotatef(180,0,1,0);
					Inimigo();
					glPopMatrix();
				}
				else if (prox == 2) {
					inimigos[i][0] = inimigos[i][0] - vel_inimigos;
					aux = inimigos[i][0] - (comprimento - km);
					glPushMatrix();
					//Leva para o centro da curva
					glTranslatef(-raio_curva - (largura / 2), 0, 0);
					//Roda junto com a pista
					glRotatef(-ang, 0, 1, 0);
					//Ir para o centro da curva a direta
					glTranslatef(0, 0, -2 * (raio_curva + (largura / 2)));
					//Roda o angulo necessário para colocar no lugar certo respeitando a distancia que falta
					glRotatef(-90 - vel_ang*aux / vel, 0, 1, 0);
					//Posiciona na curva
					glTranslatef(raio_curva + largura - inimigos[i][1], 0, 0);
					//Desenha o carro
					//glRotatef(180,0,1,0);
					Inimigo();
					glPopMatrix();
				}
			}

		}
		else if (atual == 2) {
			if (inimigos[i][0] <= comprimento - km) {
				inimigos[i][0] = inimigos[i][0] - vel_inimigos;
				glPushMatrix();
				//Leva pro centro da curva
				glTranslatef(raio_curva + (largura / 2), 0, 0);
				//Roda o angulo necessário para colocar no lugar certo respeitando a distancia que falta
				glRotatef(180 - vel_ang*inimigos[i][0] / vel, 0, 1, 0);
				//Posiciona na curva
				glTranslatef(raio_curva + largura - inimigos[i][1], 0, 0);
				//Desenha o carro
				Inimigo();
				glPopMatrix();
			}
			else {
				if (prox == 0) {
					inimigos[i][0] = inimigos[i][0] - vel_inimigos;
					aux = inimigos[i][0] - (comprimento - km);
					glPushMatrix();
					//Leva pro centro da curva
					glTranslatef(raio_curva + (largura / 2), 0, 0);
					//Roda junto com a pista
					glRotatef(ang, 0, 1, 0);
					//Vai pro fim da pista reta
					glTranslatef(aux, 0, -raio_curva - largura + inimigos[i][1]);
					//Desenha o carro
					glRotatef(90,0,1,0);
					Inimigo();
					glPopMatrix();
				}
				else if (prox == 1) {
					inimigos[i][0] = inimigos[i][0] - vel_inimigos;
					aux = inimigos[i][0] - (comprimento - km);
					glPushMatrix();
					//Leva para o centro da curva
					glTranslatef(raio_curva + (largura / 2), 0, 0);
					//Roda junto com a pista
					glRotatef(ang, 0, 1, 0);
					//Ir para o centro da curva a esquerda
					glTranslatef(0, 0, -2 * (raio_curva + (largura / 2)));
					//Roda o angulo necessário para colocar no lugar certo respeitando a distancia que falta
					glRotatef(-90 + vel_ang*aux / vel, 0, 1, 0);
					//Posiciona na curva
					glTranslatef(raio_curva + inimigos[i][1], 0, 0);
					//Desenha o carro
					glRotatef(180,0,1,0);
					Inimigo();
					glPopMatrix();
				}
				else if (prox == 2) {
					inimigos[i][0] = inimigos[i][0] - vel_inimigos;
					glPushMatrix();
					//Leva pro centro da curva
					glTranslatef(raio_curva + (largura / 2), 0, 0);
					//Roda o angulo necessário para colocar no lugar certo respeitando a distancia que falta
					glRotatef(180 - vel_ang*inimigos[i][0] / vel, 0, 1, 0);
					//Posiciona na curva
					glTranslatef(raio_curva + largura - inimigos[i][1], 0, 0);
					//Desenha o carro
					//glRotatef(180,0,1,0);
					Inimigo();
					glPopMatrix();
				}
			}
		}
	}

	if (inimigos[0][0] <= 0.5) {
		Colisao();
		if (inimigos[0][0] <= 0) {
			num_inimigos--;
		}
	}
}


void Chao() {
	glColor3ub(r_chao, g_chao, b_chao);
	glBegin(GL_QUADS);
	glVertex3f(-50, -0.01, 10);
	glVertex3f(-50, -0.01, -170);
	glVertex3f(50, -0.01, -170);
	glVertex3f(50, -0.01, 10);
	glEnd();
}

void Reta(int sentido) {
	glColor3ub(r_pista, g_pista, b_pista);
	glBegin(GL_QUADS);
	if (sentido == 1) {
		glVertex3f(0, 0, largura / 2);
		glVertex3f(0, 0, -largura / 2);
		glVertex3f(-comprimento, 0, -largura / 2);
		glVertex3f(-comprimento, 0, largura / 2);
	}
	else {
		if (atual == 0) {
			glVertex3f(largura / 2, 0, 5);
			glVertex3f(-largura / 2, 0, 5);
		}
		else {
			glVertex3f(largura / 2, 0, 0);
			glVertex3f(-largura / 2, 0, 0);
		}
		glVertex3f(-largura / 2, 0, -comprimento);
		glVertex3f(largura / 2, 0, -comprimento);

	}
	glEnd();
}

void Curva(int proximo) {

	int i;
	float ax,az,bx,bz,cx,cz,dx,dz;

	ax=raio_curva; az=0;
	bx=raio_curva+largura; 		bz=0;
	cx=(cos_ang*ax); 		cz=(sen_ang*ax+az*cos_ang);
	dx=(cos_ang*bx); 		dz=(sen_ang*bx+bz*cos_ang);
	

	if(proximo==0){
		az=-5;
		bz=-5;
	}
	glColor3ub(r_pista, g_pista, b_pista);
	for(i=0;i<numquads;i++){
		glBegin(GL_QUADS);
			glVertex3f(ax,0,az); 
			glVertex3f(bx,0,bz);
			glVertex3f(dx,0,dz); 
			glVertex3f(cx,0,cz);     
		glEnd();
		ax=cx; az=cz;
		bx=dx; 		bz=dz;
		cx=(cos_ang*ax-sen_ang*az); 		cz=(sen_ang*ax+az*cos_ang);
		dx=(cos_ang*bx-sen_ang*bz); 		dz=(sen_ang*bx+bz*cos_ang);

	}
	if(proximo==0){
		fim_pista1x=ax;
		fim_pista1z=az;
		fim_pista2x=bx;
		fim_pista2z=bz;
		az=-5;
		bz=-5;
	}


}

void Frente_Frente() {

	glPushMatrix();
	glTranslatef(0, 0, km);
	Reta(0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, km - comprimento);
	Reta(0);
	glPopMatrix();
}

void Frente_Esq() {

	glPushMatrix();
	glTranslatef(0, 0, km);
	Reta(0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-raio_curva - (largura / 2), 0, km - comprimento);
	glScalef(1, 1, -1);
	Curva(1);
	glPopMatrix();
}

void Frente_Dir() {

	glPushMatrix();
	glTranslatef(0, 0, km);
	Reta(0);
	glPopMatrix();

	glPushMatrix();
	glScalef(-1, 1, 1);
	glTranslatef(-raio_curva - (largura / 2), 0, km - comprimento);
	glScalef(1, 1, -1);
	Curva(1);
	glPopMatrix();
}

void Esq_Frente() {
	glPushMatrix();
	glScalef(1, 1, -1);
	glTranslatef(-raio_curva - (largura / 2), 0, 0);
	glRotatef(ang, 0, 1, 0);
	Curva(0);
	glTranslatef((fim_pista1x + fim_pista2x) / 2, 0, (fim_pista1z + fim_pista2z) / 2);
	Reta(1);
	glPopMatrix();
}

void Esq_Esq() {
	glPushMatrix();
	glScalef(1, 1, -1);
	glTranslatef(-raio_curva - largura / 2, 0, 0);
	glRotatef(ang, 0, 1, 0);
	Curva(0);
	glRotatef(-90, 0, 1, 0);
	Curva(1);
	glPopMatrix();
}

void Esq_Dir() {
	glPushMatrix();
	glScalef(1, 1, -1);
	glTranslatef(-raio_curva - (largura / 2), 0, 0);
	glRotatef(ang, 0, 1, 0);
	Curva(0);
	glTranslatef((fim_pista1x + fim_pista2x) / 2, 0, 2 * (fim_pista1z + fim_pista2z) / 2);
	glRotatef(180, 0, 1, 0);
	Curva(1);
	glPopMatrix();
}

void Dir_Frente() {
	glPushMatrix();
	glScalef(-1, 1, -1);
	glTranslatef(-raio_curva - (largura / 2), 0, 0);
	glRotatef(ang, 0, 1, 0);
	Curva(0);
	glTranslatef((fim_pista1x + fim_pista2x) / 2, 0, (fim_pista1z + fim_pista2z) / 2);
	Reta(1);
	glPopMatrix();
}

void Dir_Esq() {
	glPushMatrix();
	glScalef(-1, 1, -1);
	glTranslatef(-raio_curva - (largura / 2), 0, 0);
	glRotatef(ang, 0, 1, 0);
	Curva(0);
	glTranslatef((fim_pista1x + fim_pista2x) / 2, 0, 2 * (fim_pista1z + fim_pista2z) / 2);
	glRotatef(180, 0, 1, 0);
	Curva(1);
	glPopMatrix();
}

void Dir_Dir() {
	glPushMatrix();
	glScalef(-1, 1, -1);
	glTranslatef(-raio_curva - (largura / 2), 0, 0);
	glRotatef(ang, 0, 1, 0);
	Curva(0);
	glScalef(-1, 1, 1);
	Curva(1);
	glPopMatrix();
}


void Gerenciador() {
	tempo += relogio;

	if (atual == 0) {
		switch (prox) {
		case 0: Frente_Frente(); break;
		case 1: Frente_Esq(); break;
		case 2: Frente_Dir(); break;
		}
		curva = 0;
	}
	else if (atual == 1) {
		switch (prox) {
		case 0: Esq_Frente(); break;
		case 1: Esq_Esq(); break;
		case 2: Esq_Dir(); break;
		}
		curva = 1;
	}
	else {
		switch (prox) {
		case 0: Dir_Frente(); break;
		case 1: Dir_Esq(); break;
		case 2: Dir_Dir(); break;
		}
		curva = 1;
	}
	int aux1;
	if (curva == 1) {
		if (ang>90) {
			atual = prox;
			prox = rand() % 3;
			ang = 0;
			km = 0;
		}
		ang += vel_ang;
		km += vel;
	}
	else {
		if (km>comprimento) {
			aux1 = atual;
			atual = prox;
			prox = aux1;
			prox = rand() % 3;
			km = 0;
		}
		km = km + vel;
	}
}

void Ceu(){
	glBegin(GL_QUADS);
	glColor3ub(250, 2, 11);
		glVertex3f(-100,-10,-170);
		glVertex3f(100,-10,-170);
	glColor3ub(73, 14, 97);
		glVertex3f(100,10,-170);
		glVertex3f(-100,10,-1700);
	glEnd();
	glBegin(GL_QUADS);
	glColor3ub(73, 14, 97);
		glVertex3f(-100,10,-170);
		glVertex3f(100,10,-170);
	glColor3ub(0, 0, 0);
		glVertex3f(100,20,-170);
		glVertex3f(-100,20,-170);
	glEnd();
	
	glPushMatrix();
	glTranslatef(-50,0,-169);
	Skyline0();
	glTranslatef(0,0,1);
	Skyline1();
	glTranslatef(0,0,1);
	Skyline2();
	glPopMatrix();
}

void Display() {
	// Inicializa parâmetros de rendering
	// Define a cor de fundo da janela de visualização como preta
	glClearColor(r_ceu, g_ceu, b_ceu, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_SMOOTH);
	glEnable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);/*glMatrixMode()- define qual matriz será alterada. SEMPRE defina o tipo de apresentação (Ortogonal ou Perspectiva) na matriz PROJECTION.*/
	glLoadIdentity();//"Limpa" ou "transforma" a matriz em identidade, reduzindo possíveis erros.

	//if (projecao == 1)
	//	glOrtho(-400, 400, -400, 400, -400, 400); //Define a projeção como ortogonal
	//else
		gluPerspective(45, 1, 1, 180); //Define a projeção como perspectiva

	glMatrixMode(GL_MODELVIEW);/*Define qual matriz será alterada -> MODELVIEW (onde o desenho ocorrerá).*/
	glLoadIdentity(); //"Limpa" ou "transforma" a matriz em identidade.


	gluLookAt(posx, posy, posz, ox, oy, oz, lx, ly, lz); //Define a pos da câmera, para onde olha e qual eixo está na vertical.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /* " limpando os buffers para suportarem animações */
	if (vel == 0) {
		glPushMatrix();
		glColor3ub(255, 0, 0);
		//glScalef(2, 2, 2);
		drawBitmapText("Game     Over    -     Loser!", -3, 4, -10);
		glPopMatrix();
	}

	//Chamada para Função  ou funções para desenhar o objeto/cena...
	//----------------------------------------------------------------
	Ceu();
	Chao();
	Carro();
	Spawn();
	Gerenciador();
	//----------------------------------------------------------------   

	glutSwapBuffers(); //Executa a Cena. SwapBuffers dá suporte para mais de um buffer, permitindo execução de animações sem cintilações. 
	glutPostRedisplay();
}

void Idle() {

	// Pega o tempo atual desde o começo da execução (em milissegundos)
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = currentTime - lastFrameTime;
	lastFrameTime = currentTime;

	// Animação do carro se movendo pros lados
	// Tem que ser antes do Sleep para usar o deltaTime correto
	car_rot = 0;
	if (pos_car_final != pos_car) 
	{
		float distancia = (pos_car - pos_car_final);
		float direcao = distancia / fabsf(distancia);

		car_rot = -5 * direcao;
		pos_car_final += direcao * fminf(fabsf(vel_x * deltaTime), fabsf(distancia));
	}

	// Animação da nuvem
	pn_cont += 0.002 * deltaTime;
	if(pn<22) pn += 0.002 * deltaTime;
	else pn = -22;

	if (pn2<22) pn2 += 0.001 * deltaTime;
	else pn2 = -22;

	// deltaTime é o tempo entre 2 frames.
	// 16 milissegundos é o tempo aproximado entre 2 frames de um aplicativo rodando a 60FPS
	if (deltaTime < 16) {
		int sleepTime = 16 - deltaTime;
		usleep(sleepTime);
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	int aux1;
	if (key == 'd') {
		if (pos_car<(largura / 2) - 1) {
			pos_car += mov;
		}
	}
	else if (key == 'a') {
		if (pos_car>-(largura / 2) + 1) {
			pos_car -= mov;
		}
	}
	else if (key == 'p') {
		if (curva == 1) {
			if (ang>90) {
				atual = prox;
				prox = rand() % 3;
				ang = 0;
				km = 0;
			}
			ang += vel_ang;
			km += vel;
		}
		else {
			if (km>comprimento) {
				atual = prox;
				prox = rand() % 3;
				km = 0;
			}
			km = km + vel;
		}
	}
	else if (key == 'o') {
		if (curva == 1) {
			if (ang<0) {
				aux1 = prox;
				prox = atual;
				atual = aux1;
				//atual=prox;
				//prox=rand()%3;
				ang = 90;
				km = comprimento;
			}
			ang -= vel_ang;
			km -= vel;
		}
		else {
			if (km<0) {
				aux1 = prox;
				prox = atual;
				atual = aux1;
				//atual=prox;
				//prox=rand()%3;
				km = comprimento;
			}
			km = km - vel;
		}
	}
	else if (key == 'j') {
		vel = vel_p;
		vel_ang = vel_ang_p;
		vel_inimigos = vel_inimigos_p;
		mov = mov_p;
		relogio = relogio_p;
	}
	else if (key == 'c') {
		switch (camera)
		{
		case rear:
			// Muda a camera para top
			posx = 0, posy = 18, posz = 5;
			oy = 0, ox = 0, oz = -5;
			camera = top;
			break;
		case top:
			// Muda a camera pro left
			posx = -2, posy = 2, posz = 7;
			oy = 0, ox = 0, oz = -6;
			camera = left;
			break;
		case left:
			// Muda a camera para rear
			posx = 0, posy = 3, posz = 10;  //Variáveis que definem a posição da câmera 0,3,10
			oy = 0, ox = 0, oz = -10;		//Variáveis que definem para onde a câmera olha 0,0,-10
			camera = rear;
			break;
		}
	}

	glutPostRedisplay();
}


int main(int argc, char **argv) {
	//Calcula de valores para evitar repetição de passos na criação da curva
	cos_ang= cos((90*3.14)/(numquads*180));
	sen_ang= sin((90*3.14)/(numquads*180));

	raio_curva=2*comprimento/3.14; //Calculo para que a curva tenha o mesmo tamanho que a reta
	vel_ang=(vel*180)/(raio_curva*3.14); //Calculo para que a velocidade na curva seja a mesma que na reta

	vel_inimigos=vel*2.5; //Inicializa a velocidade dos inimigos

	//Salvando os valores para pause/resume
	vel_p = vel;
	vel_ang_p = vel_ang;
	vel_inimigos_p = vel_inimigos;
	mov_p = mov;
	relogio_p = relogio;

	glutInit(&argc, argv); // Inicializa glut
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); /*Define as características do espaço vetorial. Nesse caso, permite animações (sem cintilações)| cores compostas por Verm. Verde e Azul|Buffer que permite trablhar com profundidade e elimina faces escondidas.*/

	glutInitWindowSize(800, 800);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Enduro");
	glutDisplayFunc(Display);
	glutIdleFunc(Idle);
	//glutMouseFunc(Mouse);
	glutKeyboardFunc(keyboard);
	//glutSpecialFunc(TeclasEspeciais);

	lastFrameTime = glutGet(GLUT_ELAPSED_TIME);
	glutMainLoop();
	return 0;
}

void Carro(){
	glPushMatrix();
	glTranslatef(pos_car,0,0);
	glRotatef(-90,0,1,0);
	//glTranslatef(-5,0,0);
	glScalef(0.2,0.2,0.2);
	
	/**Traçado**/
	if(cor==0)
		glColor3ub(35,35,35);
	else if(cor==1)
		glColor3ub(230,0,0);
	else if(cor==2)
		glColor3ub(0,230,0);
	else if(cor==3)
		glColor3ub(0,0,230);
	else if(cor==4)
		glColor3ub(230,230,0);
	else if(cor==5)
		glColor3ub(230,0,230);
	else if(cor==6)
		glColor3ub(0,230,230);
	else if(cor==7)
		glColor3ub(230,230,230);
	glBegin(GL_LINE_STRIP);
		glVertex3f(0,2,0);
		glVertex3f(2,2.25,0);
		glVertex3f(4,2.25,0);
	glEnd();
	
	glBegin(GL_LINE_STRIP);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(4,2.25,2.5);
	glEnd();

	//Portas
	glBegin(GL_LINE_STRIP);
		glVertex3f(4,2.25,2.5);
		glVertex3f(3.5,1,2.75);
		glVertex3f(3.75,0.75,2.75);
		glVertex3f(4.25,0.5,2.5);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(7,3.25,2);
		glVertex3f(7,2,2.5);
		glVertex3f(7,1,2.75);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(3.5,1,-2.75);
		glVertex3f(3.75,0.75,-2.75);
		glVertex3f(4.25,0.5,-2.5);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(7,3.25,-2);
		glVertex3f(7,2,-2.5);
		glVertex3f(7,1,-2.75);
	glEnd();

	//Fim portas

	glBegin(GL_LINES);
		glVertex3f(5,3.25,-2);
		glVertex3f(5,3.25,2);

		glVertex3f(8,3.25,-2);
		glVertex3f(8,3.25,2);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(10,2.25,-2.5);
		glVertex3f(8,3.25,-2);
		glVertex3f(5,3.25,-2);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(2,2.5,-2.5);
		glVertex3f(0,2,-2.5);
		glVertex3f(0,2,2.5);
		glVertex3f(2,2.5,2.5);
		glVertex3f(4,2.25,2.5);
		glVertex3f(5,3.25,2);
		glVertex3f(8,3.25,2);
		glVertex3f(10,2.25,2.5);

	glEnd();

	glColor3ub(200,200,200);
	glBegin(GL_LINE_LOOP);
		glVertex3f(9.75,1,-2.75);
		glVertex3f(10,2.25,-2.5);
		glVertex3f(10,2.25,2.5);
		glVertex3f(9.75,1,2.75);
				
	glEnd();

	/**Decalque**/
	glColor3ub(255,255,255);
	glBegin(GL_QUADS);
		glVertex3f(0,2.01,0.2);
		glVertex3f(0,2.01,1.4);
		glVertex3f(2,2.51,1.4);
		glVertex3f(2,2.51,0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4,2.26,0.2);
		glVertex3f(4,2.26,1.4);
		glVertex3f(2,2.51,1.4);
		glVertex3f(2,2.51,0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(0,2.01,-0.2);
		glVertex3f(0,2.01,-1.4);
		glVertex3f(2,2.51,-1.4);
		glVertex3f(2,2.51,-0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4,2.26,-0.2);
		glVertex3f(4,2.26,-1.4);
		glVertex3f(2,2.51,-1.4);
		glVertex3f(2,2.51,-0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(5,3.26,0.2);
		glVertex3f(5,3.26,1.4);
		glVertex3f(6,3.51,1.4);
		glVertex3f(6,3.51,0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(8,3.26,0.2);
		glVertex3f(8,3.26,1.4);
		glVertex3f(6,3.51,1.4);
		glVertex3f(6,3.51,0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(5,3.26,-0.2);
		glVertex3f(5,3.26,-1.4);
		glVertex3f(6,3.51,-1.4);
		glVertex3f(6,3.51,-0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(8,3.26,-0.2);
		glVertex3f(8,3.26,-1.4);
		glVertex3f(6,3.51,-1.4);
		glVertex3f(6,3.51,-0.2);
	glEnd();

	/**Vidros**/
	glColor3ub(0,0,0);
	glBegin(GL_QUADS);
		glVertex3f(3.99,2.25,2.4);
		glVertex3f(4.99,3.25,1.9);
		glVertex3f(4.99,3.25,-1.9);
		glVertex3f(3.99,2.25,-2.4);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4.25,2.25,2.55);
		glVertex3f(5.1,3.25,2.05);
		glVertex3f(6.75,3.25,2.05);
		glVertex3f(6.75,2.25,2.55);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(8,2.25,2.55);
		glVertex3f(7.5,3,2.15);
		glVertex3f(7,3.25,2.05);
		glVertex3f(7,2.25,2.55);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4.25,2.25,-2.55);
		glVertex3f(5.1,3.25,-2.05);
		glVertex3f(6.75,3.25,-2.05);
		glVertex3f(6.75,2.25,-2.55);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(8,2.25,-2.55);
		glVertex3f(7.5,3,-2.15);
		glVertex3f(7,3.25,-2.05);
		glVertex3f(7,2.25,-2.55);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(9.99,2.26,2.3);
		glVertex3f(8.01,3.25,1.8);
		glVertex3f(8.01,3.25,-1.8);
		glVertex3f(9.99,2.26,-2.3);
	glEnd();

	/**Placa**/
	glColor3ub(200,200,200);
	glBegin(GL_QUADS);
		glVertex3f(10.01,2,-2);
		glVertex3f(10.01,2,2);
		glVertex3f(9.76,1.5,2);
		glVertex3f(9.76,1.5,-2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(9.9,1.5,-1);
		glVertex3f(9.9,1.5,1);
		glVertex3f(9.76,1.25,1);
		glVertex3f(9.76,1.25,-1);
	glEnd();


	/**Faróis**/	
	glColor3ub(90,20,0);
	glBegin(GL_QUADS);
		glVertex3f(10.01,2.25,2.5);
		glVertex3f(10.01,2.25,1.75);
		glVertex3f(9.76,1,2);
		glVertex3f(9.76,1,2.75);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(10.01,2.25,-2.5);
		glVertex3f(10.01,2.25,-1.75);
		glVertex3f(9.76,1,-2);
		glVertex3f(9.76,1,-2.75);
	glEnd();


	glPushMatrix();
	glTranslatef(-0.01,1.5,2);
	glRotatef(90,0,1,0);
	glColor3ub(230,230,230);
	gluDisk(gluNewQuadric(),0.25,0.3,20,20);
	glColor3ub(255,255,150);
	gluDisk(gluNewQuadric(),0,0.25,20,20);
	glTranslatef(0.5,0,0);
	glColor3ub(230,230,230);
	gluDisk(gluNewQuadric(),0.1,0.15,20,20);
	glColor3ub(255,255,150);
	gluDisk(gluNewQuadric(),0,0.1,20,20);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.01,1.5,-2);
	glRotatef(90,0,1,0);
	glColor3ub(230,230,230);
	gluDisk(gluNewQuadric(),0.25,0.3,20,20);
	glColor3ub(255,255,150);
	gluDisk(gluNewQuadric(),0,0.25,20,20);
	glTranslatef(-0.5,0,0);
	glColor3ub(230,230,230);
	gluDisk(gluNewQuadric(),0.1,0.15,20,20);
	glColor3ub(255,255,150);
	gluDisk(gluNewQuadric(),0,0.1,20,20);
	glPopMatrix();


	/**Base**/
	if(cor==0)
		glColor3ub(0,0,0);
	else if(cor==1)
		glColor3ub(255,0,0);
	else if(cor==2)
		glColor3ub(0,255,0);
	else if(cor==3)
		glColor3ub(0,0,255);
	else if(cor==4)
		glColor3ub(255,255,0);
	else if(cor==5)
		glColor3ub(255,0,255);
	else if(cor==6)
		glColor3ub(0,255,255);
	else if(cor==7)
		glColor3ub(255,255,255);
	
	//Lado Direitod
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,2.75);
		glVertex3f(1,0.75,2.75);
		glVertex3f(0.75,0.5,2.5);
		glVertex3f(0.25,0.5,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(6.25,0.75,2.75);
		glVertex3f(2.75,0.75,2.75);
		glVertex3f(3,0.5,2.5);
		glVertex3f(6,0.5,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(7.75,0.75,2.75);
		glVertex3f(9.5,0.75,2.75);
		glVertex3f(9,0.5,2.5);
		glVertex3f(8,0.5,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,2.75);
		glVertex3f(0,1,2.75);
		glVertex3f(2,1,2.75);
		glVertex3f(1,0.75,2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(2,1,2.75);
		glVertex3f(2.75,0.75,2.75);
		glVertex3f(6.25,0.75,2.75);
		glVertex3f(7,1,2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(7.75,0.75,2.75);
		glVertex3f(9.5,0.75,2.75);
		glVertex3f(9.75,1,2.75);
		glVertex3f(7,1,2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,1,2.75);
		glVertex3f(0,2,2.5);
		glVertex3f(9.75,2,2.5);
		glVertex3f(9.75,1,2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,2,2.5);
		glVertex3f(2,2.5,2.5);
		glVertex3f(4,2.25,2.5);
		glVertex3f(4,2,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(9.75,2,2.5);
		glVertex3f(10,2.25,2.5);
		glVertex3f(4,2.25,2.5);
		glVertex3f(4,2,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(8,3.25,2);
		glVertex3f(5,3.25,2);
		glVertex3f(4,2.25,2.5);
		glVertex3f(10,2.25,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(5,3.25,2);
		glVertex3f(6,3.5,1);
		glVertex3f(8,3.25,2);
		glVertex3f(7,3.25,2);
	glEnd();

	glBegin(GL_TRIANGLES);
		glVertex3f(10,2.25,2.5);
		glVertex3f(9.75,2,2.5);
		glVertex3f(9.75,1,2.75);
	glEnd();
	
	
	//Lado Esquerdo
	
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,-2.75);
		glVertex3f(1,0.75,-2.75);
		glVertex3f(0.75,0.5,-2.5);
		glVertex3f(0.25,0.5,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(6.25,0.75,-2.75);
		glVertex3f(2.75,0.75,-2.75);
		glVertex3f(3,0.5,-2.5);
		glVertex3f(6,0.5,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(7.75,0.75,-2.75);
		glVertex3f(9.5,0.75,-2.75);
		glVertex3f(9,0.5,-2.5);
		glVertex3f(8,0.5,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,-2.75);
		glVertex3f(0,1,-2.75);
		glVertex3f(2,1,-2.75);
		glVertex3f(1,0.75,-2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(2,1,-2.75);
		glVertex3f(2.75,0.75,-2.75);
		glVertex3f(6.25,0.75,-2.75);
		glVertex3f(7,1,-2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(7.75,0.75,-2.75);
		glVertex3f(9.5,0.75,-2.75);
		glVertex3f(9.75,1,-2.75);
		glVertex3f(7,1,-2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,1,-2.75);
		glVertex3f(0,2,-2.5);
		glVertex3f(9.75,2,-2.5);
		glVertex3f(9.75,1,-2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,2,-2.5);
		glVertex3f(2,2.5,-2.5);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(4,2,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(9.75,2,-2.5);
		glVertex3f(10,2.25,-2.5);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(4,2,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(8,3.25,-2);
		glVertex3f(5,3.25,-2);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(10,2.25,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(5,3.25,-2);
		glVertex3f(6,3.5,-1);
		glVertex3f(8,3.25,-2);
		glVertex3f(7,3.25,-2);
	glEnd();

	glBegin(GL_TRIANGLES);
		glVertex3f(10,2.25,-2.5);
		glVertex3f(9.75,2,-2.5);
		glVertex3f(9.75,1,-2.75);
	glEnd();
	
	
	

	//Parabrisa
	glBegin(GL_QUADS);
		glVertex3f(4,2.25,2.5);
		glVertex3f(5,3.25,2);
		glVertex3f(5,3.25,-2);
		glVertex3f(4,2.25,-2.5);
	glEnd();


	//Teto
	glBegin(GL_QUADS);
		glVertex3f(6,3.5,1);
		glVertex3f(5,3.25,2);
		glVertex3f(5,3.25,-2);
		glVertex3f(6,3.5,-1);
	glEnd();
	glBegin(GL_QUADS);
		glVertex3f(6,3.5,1);
		glVertex3f(8,3.25,2);
		glVertex3f(8,3.25,-2);
		glVertex3f(6,3.5,-1);
	glEnd();

	//Parabrisa Traseiro
	glBegin(GL_QUADS);
		glVertex3f(10,2.25,2.5);
		glVertex3f(8,3.25,2);
		glVertex3f(8,3.25,-2);
		glVertex3f(10,2.25,-2.5);
	glEnd();

	//Porta-Malas
	//glColor3ub(200,0,0);
	glBegin(GL_QUADS);
		glVertex3f(10,2.25,2.5);
		glVertex3f(9.75,1,2.75);
		glVertex3f(9.75,1,-2.75);
		glVertex3f(10,2.25,-2.5);
	glEnd();



	glBegin(GL_QUADS);
		glVertex3f(9.5,0.75,2.75);
		glVertex3f(9,0.5,2.5);
		glVertex3f(9,0.5,-2.5);
		glVertex3f(9.5,0.75,-2.75);
	glEnd();

	//Capô
	glBegin(GL_QUADS);
		glVertex3f(4,2.25,2.5);
		glVertex3f(2,2.5,2.5);
		glVertex3f(2,2.5,0);
		glVertex3f(4,2.25,0);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(0,2,2.5);
		glVertex3f(2,2.5,2.5);
		glVertex3f(2,2.5,0);
		glVertex3f(0,2,0);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(2,2.5,-2.5);
		glVertex3f(2,2.5,0);
		glVertex3f(4,2.25,0);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(0,2,-2.5);
		glVertex3f(2,2.5,-2.5);
		glVertex3f(2,2.5,0);
		glVertex3f(0,2,0);
	glEnd();

	//Para-choque
	glBegin(GL_QUADS);
		glVertex3f(0,2,-2.5);
		glVertex3f(0,1,-2.75);
		glVertex3f(0,1,2.75);
		glVertex3f(0,2,2.5);
	glEnd();

	glColor3ub(230,230,230);
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,-2.75);
		glVertex3f(0,1,-2.75);
		glVertex3f(0,1,2.75);
		glVertex3f(0,0.75,2.75);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(0,0.75,-2.75);
		glVertex3f(0.25,0.5,-2.5);
		glVertex3f(0.25,0.5,2.5);
		glVertex3f(0,0.75,2.75);
	glEnd();

	//Paralama
	glColor3ub(220,220,220);
	glBegin(GL_QUADS);
		glVertex3f(9.5,0.75,2.75);
		glVertex3f(9.75,1,2.75);
		glVertex3f(9.75,1,-2.75);
		glVertex3f(9.5,0.75,-2.75);
	glEnd();

	//Pneus
	glColor3ub(10,10,10);
	glPushMatrix();
	glTranslatef(2,1,1.5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 20, 20);//Pneu
	glTranslatef(0,0,1);
	gluDisk(gluNewQuadric(),0.5,1,20,20);//Pneu
	glColor3ub(200,200,200);
	gluDisk(gluNewQuadric(),0,0.5,20,20);//Calota
	glPopMatrix();

	glColor3ub(10,10,10);
	glPushMatrix();
	glTranslatef(2,1,-2.5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 20, 20);//Pneu
	gluDisk(gluNewQuadric(),0.5,1,20,20);//Pneu
	glColor3ub(200,200,200);
	gluDisk(gluNewQuadric(),0,0.5,20,20);//Calota
	glPopMatrix();

	glColor3ub(10,10,10);
	glPushMatrix();
	glTranslatef(7,1,1.5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 20, 20);//Pneu
	glTranslatef(0,0,1);
	gluDisk(gluNewQuadric(),0.5,1,20,20);//Pneu
	glColor3ub(200,200,200);
	gluDisk(gluNewQuadric(),0,0.5,20,20);//Calota
	glPopMatrix();

	glColor3ub(10,10,10);
	glPushMatrix();
	glTranslatef(7,1,-2.5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 20, 20);//Pneu
	gluDisk(gluNewQuadric(),0.5,1,20,20);//Pneu
	glColor3ub(200,200,200);
	gluDisk(gluNewQuadric(),0,0.5,20,20);//Calota
	glPopMatrix();

	glPopMatrix();	
}

void Inimigo(){
	glPushMatrix();
	glRotatef(-90,0,1,0);
	glScalef(0.2,0.2,0.2);
	
	/**Traçado**/
	if(cor_i==0)
		glColor3ub(35,35,35);
	else if(cor_i==1)
		glColor3ub(230,0,0);
	else if(cor_i==2)
		glColor3ub(0,230,0);
	else if(cor_i==3)
		glColor3ub(0,0,230);
	else if(cor_i==4)
		glColor3ub(230,230,0);
	else if(cor_i==5)
		glColor3ub(230,0,230);
	else if(cor_i==6)
		glColor3ub(0,230,230);
	else if(cor_i==7)
		glColor3ub(230,230,230);
	glBegin(GL_LINE_STRIP);
		glVertex3f(0,2,0);
		glVertex3f(2,2.25,0);
		glVertex3f(4,2.25,0);
	glEnd();
	
	glBegin(GL_LINE_STRIP);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(4,2.25,2.5);
	glEnd();

	//Portas
	glBegin(GL_LINE_STRIP);
		glVertex3f(4,2.25,2.5);
		glVertex3f(3.5,1,2.75);
		glVertex3f(3.75,0.75,2.75);
		glVertex3f(4.25,0.5,2.5);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(7,3.25,2);
		glVertex3f(7,2,2.5);
		glVertex3f(7,1,2.75);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(3.5,1,-2.75);
		glVertex3f(3.75,0.75,-2.75);
		glVertex3f(4.25,0.5,-2.5);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(7,3.25,-2);
		glVertex3f(7,2,-2.5);
		glVertex3f(7,1,-2.75);
	glEnd();

	//Fim portas

	glBegin(GL_LINES);
		glVertex3f(5,3.25,-2);
		glVertex3f(5,3.25,2);

		glVertex3f(8,3.25,-2);
		glVertex3f(8,3.25,2);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(10,2.25,-2.5);
		glVertex3f(8,3.25,-2);
		glVertex3f(5,3.25,-2);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(2,2.5,-2.5);
		glVertex3f(0,2,-2.5);
		glVertex3f(0,2,2.5);
		glVertex3f(2,2.5,2.5);
		glVertex3f(4,2.25,2.5);
		glVertex3f(5,3.25,2);
		glVertex3f(8,3.25,2);
		glVertex3f(10,2.25,2.5);

	glEnd();

	glColor3ub(200,200,200);
	glBegin(GL_LINE_LOOP);
		glVertex3f(9.75,1,-2.75);
		glVertex3f(10,2.25,-2.5);
		glVertex3f(10,2.25,2.5);
		glVertex3f(9.75,1,2.75);
				
	glEnd();

	/**Decalque**/
	glColor3ub(255,255,255);
	glBegin(GL_QUADS);
		glVertex3f(0,2.01,0.2);
		glVertex3f(0,2.01,1.4);
		glVertex3f(2,2.51,1.4);
		glVertex3f(2,2.51,0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4,2.26,0.2);
		glVertex3f(4,2.26,1.4);
		glVertex3f(2,2.51,1.4);
		glVertex3f(2,2.51,0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(0,2.01,-0.2);
		glVertex3f(0,2.01,-1.4);
		glVertex3f(2,2.51,-1.4);
		glVertex3f(2,2.51,-0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4,2.26,-0.2);
		glVertex3f(4,2.26,-1.4);
		glVertex3f(2,2.51,-1.4);
		glVertex3f(2,2.51,-0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(5,3.26,0.2);
		glVertex3f(5,3.26,1.4);
		glVertex3f(6,3.51,1.4);
		glVertex3f(6,3.51,0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(8,3.26,0.2);
		glVertex3f(8,3.26,1.4);
		glVertex3f(6,3.51,1.4);
		glVertex3f(6,3.51,0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(5,3.26,-0.2);
		glVertex3f(5,3.26,-1.4);
		glVertex3f(6,3.51,-1.4);
		glVertex3f(6,3.51,-0.2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(8,3.26,-0.2);
		glVertex3f(8,3.26,-1.4);
		glVertex3f(6,3.51,-1.4);
		glVertex3f(6,3.51,-0.2);
	glEnd();

	/**Vidros**/
	glColor3ub(0,0,0);
	glBegin(GL_QUADS);
		glVertex3f(3.99,2.25,2.4);
		glVertex3f(4.99,3.25,1.9);
		glVertex3f(4.99,3.25,-1.9);
		glVertex3f(3.99,2.25,-2.4);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4.25,2.25,2.55);
		glVertex3f(5.1,3.25,2.05);
		glVertex3f(6.75,3.25,2.05);
		glVertex3f(6.75,2.25,2.55);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(8,2.25,2.55);
		glVertex3f(7.5,3,2.15);
		glVertex3f(7,3.25,2.05);
		glVertex3f(7,2.25,2.55);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4.25,2.25,-2.55);
		glVertex3f(5.1,3.25,-2.05);
		glVertex3f(6.75,3.25,-2.05);
		glVertex3f(6.75,2.25,-2.55);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(8,2.25,-2.55);
		glVertex3f(7.5,3,-2.15);
		glVertex3f(7,3.25,-2.05);
		glVertex3f(7,2.25,-2.55);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(9.99,2.26,2.3);
		glVertex3f(8.01,3.25,1.8);
		glVertex3f(8.01,3.25,-1.8);
		glVertex3f(9.99,2.26,-2.3);
	glEnd();

	/**Placa**/
	glColor3ub(200,200,200);
	glBegin(GL_QUADS);
		glVertex3f(10.01,2,-2);
		glVertex3f(10.01,2,2);
		glVertex3f(9.76,1.5,2);
		glVertex3f(9.76,1.5,-2);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(9.9,1.5,-1);
		glVertex3f(9.9,1.5,1);
		glVertex3f(9.76,1.25,1);
		glVertex3f(9.76,1.25,-1);
	glEnd();


	/**Faróis**/	
	glColor3ub(90,20,0);
	glBegin(GL_QUADS);
		glVertex3f(10.01,2.25,2.5);
		glVertex3f(10.01,2.25,1.75);
		glVertex3f(9.76,1,2);
		glVertex3f(9.76,1,2.75);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(10.01,2.25,-2.5);
		glVertex3f(10.01,2.25,-1.75);
		glVertex3f(9.76,1,-2);
		glVertex3f(9.76,1,-2.75);
	glEnd();


	glPushMatrix();
	glTranslatef(-0.01,1.5,2);
	glRotatef(90,0,1,0);
	glColor3ub(230,230,230);
	gluDisk(gluNewQuadric(),0.25,0.3,20,20);
	glColor3ub(255,255,150);
	gluDisk(gluNewQuadric(),0,0.25,20,20);
	glTranslatef(0.5,0,0);
	glColor3ub(230,230,230);
	gluDisk(gluNewQuadric(),0.1,0.15,20,20);
	glColor3ub(255,255,150);
	gluDisk(gluNewQuadric(),0,0.1,20,20);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.01,1.5,-2);
	glRotatef(90,0,1,0);
	glColor3ub(230,230,230);
	gluDisk(gluNewQuadric(),0.25,0.3,20,20);
	glColor3ub(255,255,150);
	gluDisk(gluNewQuadric(),0,0.25,20,20);
	glTranslatef(-0.5,0,0);
	glColor3ub(230,230,230);
	gluDisk(gluNewQuadric(),0.1,0.15,20,20);
	glColor3ub(255,255,150);
	gluDisk(gluNewQuadric(),0,0.1,20,20);
	glPopMatrix();


	/**Base**/
	if(cor_i==0)
		glColor3ub(0,0,0);
	else if(cor_i==1)
		glColor3ub(255,0,0);
	else if(cor_i==2)
		glColor3ub(0,255,0);
	else if(cor_i==3)
		glColor3ub(0,0,255);
	else if(cor_i==4)
		glColor3ub(255,255,0);
	else if(cor_i==5)
		glColor3ub(255,0,255);
	else if(cor_i==6)
		glColor3ub(0,255,255);
	else if(cor_i==7)
		glColor3ub(255,255,255);
	
	//Lado Direitod
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,2.75);
		glVertex3f(1,0.75,2.75);
		glVertex3f(0.75,0.5,2.5);
		glVertex3f(0.25,0.5,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(6.25,0.75,2.75);
		glVertex3f(2.75,0.75,2.75);
		glVertex3f(3,0.5,2.5);
		glVertex3f(6,0.5,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(7.75,0.75,2.75);
		glVertex3f(9.5,0.75,2.75);
		glVertex3f(9,0.5,2.5);
		glVertex3f(8,0.5,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,2.75);
		glVertex3f(0,1,2.75);
		glVertex3f(2,1,2.75);
		glVertex3f(1,0.75,2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(2,1,2.75);
		glVertex3f(2.75,0.75,2.75);
		glVertex3f(6.25,0.75,2.75);
		glVertex3f(7,1,2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(7.75,0.75,2.75);
		glVertex3f(9.5,0.75,2.75);
		glVertex3f(9.75,1,2.75);
		glVertex3f(7,1,2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,1,2.75);
		glVertex3f(0,2,2.5);
		glVertex3f(9.75,2,2.5);
		glVertex3f(9.75,1,2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,2,2.5);
		glVertex3f(2,2.5,2.5);
		glVertex3f(4,2.25,2.5);
		glVertex3f(4,2,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(9.75,2,2.5);
		glVertex3f(10,2.25,2.5);
		glVertex3f(4,2.25,2.5);
		glVertex3f(4,2,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(8,3.25,2);
		glVertex3f(5,3.25,2);
		glVertex3f(4,2.25,2.5);
		glVertex3f(10,2.25,2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(5,3.25,2);
		glVertex3f(6,3.5,1);
		glVertex3f(8,3.25,2);
		glVertex3f(7,3.25,2);
	glEnd();

	glBegin(GL_TRIANGLES);
		glVertex3f(10,2.25,2.5);
		glVertex3f(9.75,2,2.5);
		glVertex3f(9.75,1,2.75);
	glEnd();
	
	
	//Lado Esquerdo
	
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,-2.75);
		glVertex3f(1,0.75,-2.75);
		glVertex3f(0.75,0.5,-2.5);
		glVertex3f(0.25,0.5,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(6.25,0.75,-2.75);
		glVertex3f(2.75,0.75,-2.75);
		glVertex3f(3,0.5,-2.5);
		glVertex3f(6,0.5,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(7.75,0.75,-2.75);
		glVertex3f(9.5,0.75,-2.75);
		glVertex3f(9,0.5,-2.5);
		glVertex3f(8,0.5,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,-2.75);
		glVertex3f(0,1,-2.75);
		glVertex3f(2,1,-2.75);
		glVertex3f(1,0.75,-2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(2,1,-2.75);
		glVertex3f(2.75,0.75,-2.75);
		glVertex3f(6.25,0.75,-2.75);
		glVertex3f(7,1,-2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(7.75,0.75,-2.75);
		glVertex3f(9.5,0.75,-2.75);
		glVertex3f(9.75,1,-2.75);
		glVertex3f(7,1,-2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,1,-2.75);
		glVertex3f(0,2,-2.5);
		glVertex3f(9.75,2,-2.5);
		glVertex3f(9.75,1,-2.75);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0,2,-2.5);
		glVertex3f(2,2.5,-2.5);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(4,2,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(9.75,2,-2.5);
		glVertex3f(10,2.25,-2.5);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(4,2,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(8,3.25,-2);
		glVertex3f(5,3.25,-2);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(10,2.25,-2.5);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(5,3.25,-2);
		glVertex3f(6,3.5,-1);
		glVertex3f(8,3.25,-2);
		glVertex3f(7,3.25,-2);
	glEnd();

	glBegin(GL_TRIANGLES);
		glVertex3f(10,2.25,-2.5);
		glVertex3f(9.75,2,-2.5);
		glVertex3f(9.75,1,-2.75);
	glEnd();
	
	
	

	//Parabrisa
	glBegin(GL_QUADS);
		glVertex3f(4,2.25,2.5);
		glVertex3f(5,3.25,2);
		glVertex3f(5,3.25,-2);
		glVertex3f(4,2.25,-2.5);
	glEnd();


	//Teto
	glBegin(GL_QUADS);
		glVertex3f(6,3.5,1);
		glVertex3f(5,3.25,2);
		glVertex3f(5,3.25,-2);
		glVertex3f(6,3.5,-1);
	glEnd();
	glBegin(GL_QUADS);
		glVertex3f(6,3.5,1);
		glVertex3f(8,3.25,2);
		glVertex3f(8,3.25,-2);
		glVertex3f(6,3.5,-1);
	glEnd();

	//Parabrisa Traseiro
	glBegin(GL_QUADS);
		glVertex3f(10,2.25,2.5);
		glVertex3f(8,3.25,2);
		glVertex3f(8,3.25,-2);
		glVertex3f(10,2.25,-2.5);
	glEnd();

	//Porta-Malas
	//glColor3ub(200,0,0);
	glBegin(GL_QUADS);
		glVertex3f(10,2.25,2.5);
		glVertex3f(9.75,1,2.75);
		glVertex3f(9.75,1,-2.75);
		glVertex3f(10,2.25,-2.5);
	glEnd();



	glBegin(GL_QUADS);
		glVertex3f(9.5,0.75,2.75);
		glVertex3f(9,0.5,2.5);
		glVertex3f(9,0.5,-2.5);
		glVertex3f(9.5,0.75,-2.75);
	glEnd();

	//Capô
	glBegin(GL_QUADS);
		glVertex3f(4,2.25,2.5);
		glVertex3f(2,2.5,2.5);
		glVertex3f(2,2.5,0);
		glVertex3f(4,2.25,0);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(0,2,2.5);
		glVertex3f(2,2.5,2.5);
		glVertex3f(2,2.5,0);
		glVertex3f(0,2,0);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(4,2.25,-2.5);
		glVertex3f(2,2.5,-2.5);
		glVertex3f(2,2.5,0);
		glVertex3f(4,2.25,0);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(0,2,-2.5);
		glVertex3f(2,2.5,-2.5);
		glVertex3f(2,2.5,0);
		glVertex3f(0,2,0);
	glEnd();

	//Para-choque
	glBegin(GL_QUADS);
		glVertex3f(0,2,-2.5);
		glVertex3f(0,1,-2.75);
		glVertex3f(0,1,2.75);
		glVertex3f(0,2,2.5);
	glEnd();

	glColor3ub(230,230,230);
	glBegin(GL_QUADS);
		glVertex3f(0,0.75,-2.75);
		glVertex3f(0,1,-2.75);
		glVertex3f(0,1,2.75);
		glVertex3f(0,0.75,2.75);
	glEnd();

	glBegin(GL_QUADS);
		glVertex3f(0,0.75,-2.75);
		glVertex3f(0.25,0.5,-2.5);
		glVertex3f(0.25,0.5,2.5);
		glVertex3f(0,0.75,2.75);
	glEnd();

	//Paralama
	glColor3ub(220,220,220);
	glBegin(GL_QUADS);
		glVertex3f(9.5,0.75,2.75);
		glVertex3f(9.75,1,2.75);
		glVertex3f(9.75,1,-2.75);
		glVertex3f(9.5,0.75,-2.75);
	glEnd();

	//Pneus
	glColor3ub(10,10,10);
	glPushMatrix();
	glTranslatef(2,1,1.5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 20, 20);//Pneu
	glTranslatef(0,0,1);
	gluDisk(gluNewQuadric(),0.5,1,20,20);//Pneu
	glColor3ub(200,200,200);
	gluDisk(gluNewQuadric(),0,0.5,20,20);//Calota
	glPopMatrix();

	glColor3ub(10,10,10);
	glPushMatrix();
	glTranslatef(2,1,-2.5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 20, 20);//Pneu
	gluDisk(gluNewQuadric(),0.5,1,20,20);//Pneu
	glColor3ub(200,200,200);
	gluDisk(gluNewQuadric(),0,0.5,20,20);//Calota
	glPopMatrix();

	glColor3ub(10,10,10);
	glPushMatrix();
	glTranslatef(7,1,1.5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 20, 20);//Pneu
	glTranslatef(0,0,1);
	gluDisk(gluNewQuadric(),0.5,1,20,20);//Pneu
	glColor3ub(200,200,200);
	gluDisk(gluNewQuadric(),0,0.5,20,20);//Calota
	glPopMatrix();

	glColor3ub(10,10,10);
	glPushMatrix();
	glTranslatef(7,1,-2.5);
	gluCylinder(gluNewQuadric(), 1, 1, 1, 20, 20);//Pneu
	gluDisk(gluNewQuadric(),0.5,1,20,20);//Pneu
	glColor3ub(200,200,200);
	gluDisk(gluNewQuadric(),0,0.5,20,20);//Calota
	glPopMatrix();

	glPopMatrix();	
}

void Skyline0(){	
	glColor3ub(15, 0, 36);
	
	glBegin(GL_POLYGON);
		glVertex3f(0,6.5,0);
		glVertex3f(1,6.5,0);
		glVertex3f(1,6,0);
		glVertex3f(2,6,0);
		glVertex3f(2,5.5,0);
		glVertex3f(4,5.5,0);
		glVertex3f(4,0,0);
		glVertex3f(0,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(4,0,0);
		glVertex3f(4,7,0);
		glVertex3f(5,8.5,0);
		glVertex3f(7,9.5,0);
		glVertex3f(9,10,0);
		glVertex3f(9,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(9,0,0);
		glVertex3f(9,3.5,0);
		glVertex3f(10,3.5,0);
		glVertex3f(10,4,0);
		glVertex3f(13,4,0);
		glVertex3f(13,3.5,0);
		glVertex3f(15,3.5,0);
		glVertex3f(15,2.5,0);
		glVertex3f(15.5,2.5,0);
		glVertex3f(15.5,2,0);
		glVertex3f(16,2,0);
		glVertex3f(16,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(16,0,0);
		glVertex3f(16,9.5,0);
		glVertex3f(21,9.5,0);
		glVertex3f(21,7,0);
		glVertex3f(24,7,0);
		glVertex3f(24,6.5,0);
		glVertex3f(25,6.5,0);
		glVertex3f(25,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(27,0,0);
		glVertex3f(27,1,0);
		glVertex3f(28,1,0);
		glVertex3f(28,6,0);
		glVertex3f(32,6,0);
		glVertex3f(32,5.5,0);
		glVertex3f(33,5.5,0);
		glVertex3f(34,4.5,0);
		glVertex3f(34,1,0);
		glVertex3f(36,1,0);
		glVertex3f(36,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(36,0,0);
		glVertex3f(36,8,0);
		glVertex3f(38,9,0);
		glVertex3f(39,9,0);
		glVertex3f(39,10,0);
		glVertex3f(42,10,0);
		glVertex3f(42,7,0);
		glVertex3f(43,7,0);
		glVertex3f(43,5.5,0);
		glVertex3f(45,5.5,0);
		glVertex3f(45,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(45,0,0);
		glVertex3f(45,8.5,0);
		glVertex3f(46,8.5,0);
		glVertex3f(46,10.5,0);
		glVertex3f(51,10.5,0);
		glVertex3f(51,6,0);
		glVertex3f(53,6,0);
		glVertex3f(53,5.5,0);
		glVertex3f(54,5.5,0);
		glVertex3f(54,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(54,0,0);
		glVertex3f(54,7,0);
		glVertex3f(57,7,0);
		glVertex3f(57,6,0);
		glVertex3f(58,6,0);
		glVertex3f(58,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(58,0,0);
		glVertex3f(58,10,0);
		glVertex3f(59,11,0);
		glVertex3f(61,12,0);
		glVertex3f(62.25,12,0);
		glVertex3f(62.25,14,0);
		glVertex3f(62.75,14,0);
		glVertex3f(62.75,12,0);
		glVertex3f(63,12,0);
		glVertex3f(65,11,0);
		glVertex3f(66,10,0);
		glVertex3f(66,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(66,0,0);
		glVertex3f(66,13,0);
		glVertex3f(70,13,0);
		glVertex3f(70,11.5,0);
		glVertex3f(72,11.5,0);
		glVertex3f(72,4.5,0);
		glVertex3f(73,4.5,0);
		glVertex3f(73,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(73,0,0);
		glVertex3f(73,6,0);
		glVertex3f(74,6,0);
		glVertex3f(74,8,0);
		glVertex3f(76,8,0);
		glVertex3f(76,6,0);
		glVertex3f(77,6,0);
		glVertex3f(77,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(77,0,0);
		glVertex3f(77,7.5,0);
		glVertex3f(79,7.5,0);
		glVertex3f(79,8,0);
		glVertex3f(81,8,0);
		glVertex3f(81,7.5,0);
		glVertex3f(81,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(83,0,0);
		glVertex3f(83,4,0);
		glVertex3f(85,4,0);
		glVertex3f(85,5.5,0);
		glVertex3f(86,6,0);
		glVertex3f(87,5.5,0);
		glVertex3f(88,7.5,0);
		glVertex3f(88,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(88,0,0);
		glVertex3f(88,7,0);
		glVertex3f(89,7,0);
		glVertex3f(89,8,0);
		glVertex3f(90,9,0);
		glVertex3f(91,8,0);
		glVertex3f(91,7,0);
		glVertex3f(92,7,0);
		glVertex3f(94,7,0);
		glVertex3f(94,0,0);
	glEnd();
}

void Skyline1(){	
	glColor3ub(66, 32, 88);
	
	glBegin(GL_POLYGON);
		glVertex3f(0,7,0);
		glVertex3f(2,7,0);
		glVertex3f(2,6.5,0);
		glVertex3f(3,6.5,0);
		glVertex3f(3,0,0);
		glVertex3f(0,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(32,0,0);
		glVertex3f(32,13.5,0);
		glVertex3f(33,14,0);
		glVertex3f(35,14,0);
		glVertex3f(36,13.5,0);
		glVertex3f(36,13,0);
		glVertex3f(37.5,12.5,0);
		glVertex3f(38,12.5,0);
		glVertex3f(38,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(38,0,0);
		glVertex3f(38,14,0);
		glVertex3f(39,15.5,0);
		glVertex3f(40,16,0);
		glVertex3f(41,16,0);
		glVertex3f(42,15.5,0);
		glVertex3f(43,14,0);
		glVertex3f(43,4,0);
		glVertex3f(46,4,0);
		glVertex3f(46,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(46,0,0);
		glVertex3f(46,12,0)
		glVertex3f(51,12,0)
		glVertex3f(51,0,0)
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(79,0,0);
		glVertex3f(79,19.5,0);
		glVertex3f(84,19.5,0);
		glVertex3f(84,5,0);
		glVertex3f(93,5,0);
		glVertex3f(93,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(93,0,0);
		glVertex3f(93,10.5,0);
		glVertex3f(94,12,0);
		glVertex3f(95,12,0);
		glVertex3f(96,10.5,0);
		glVertex3f(96,7.5,0);
		glVertex3f(100,7.5,0);
		glVertex3f(100,0,0);
	glEnd();
}

void Skyline2(){	
	glColor3ub(69, 33, 79);
	
	glBegin(GL_POLYGON);
		glVertex3f(3,0,0);
		glVertex3f(3,8.5,0);
		glVertex3f(7,8.5,0);
		glVertex3f(7,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(10,0,0);
		glVertex3f(10,7.5,0);
		glVertex3f(12,7.5,0);
		glVertex3f(12,11,0);
		glVertex3f(14,11,0);
		glVertex3f(14,10,0);
		glVertex3f(15,10,0);
		glVertex3f(15,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(18,0,0);
		glVertex3f(18,11.5,0);
		glVertex3f(19,12,0);
		glVertex3f(19,15,0);
		glVertex3f(25,15,0);
		glVertex3f(25,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(28,0,0);
		glVertex3f(28,15,0);
		glVertex3f(29,15,0);
		glVertex3f(29,15.5,0);
		glVertex3f(31,15.5,0);
		glVertex3f(31,13,0);
		glVertex3f(32,13,0);
		glVertex3f(32,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(35,0,0);
		glVertex3f(35,20,0);
		glVertex3f(37,20,0);
		glVertex3f(37,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(48,0,0);
		glVertex3f(48,14.5,0);
		glVertex3f(53,14.5,0);
		glVertex3f(53,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(55.5,0,0);
		glVertex3f(55.5,12,0);
		glVertex3f(56,12,0);
		glVertex3f(56,15,0);
		glVertex3f(59,15,0);
		glVertex3f(59,13.5,0);
		glVertex3f(59.5,13.5,0);
		glVertex3f(59.5,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(58,0,0);
		glVertex3f(58,19.5,0);
		glVertex3f(59,19.5,0);
		glVertex3f(59,20,0);
		glVertex3f(62,20,0);
		glVertex3f(62,19.5,0);
		glVertex3f(63,19.5,0);
		glVertex3f(63,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(67,0,0);
		glVertex3f(67,14,0);
		glVertex3f(70,14,0);
		glVertex3f(70,11,0);
		glVertex3f(71,11,0);
		glVertex3f(71,0,0);
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(73,0,0);
		glVertex3f(73,11,0);
		glVertex3f(77,11,0);
		glVertex3f(77,0,0);
	glEnd();

}