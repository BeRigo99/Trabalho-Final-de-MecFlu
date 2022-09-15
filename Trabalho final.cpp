#include <stdio.h>
#include <math.h>

#define VER 1.4
/*
    perda de carga no trocador de calor incluida
    matrizes de numero de valvulas, joelhos etc. removidas
    constantes de perda para joelhos, valvulas e trocador de calor agora nos defines
    totalcost agora incluso
    perdas na tubulacao interna do ar condicionado inclusa na perda de carga das tubulacoes A e B
    sistema para identificar custo mais baixo adicionado
*/

#define PI 3.14159265
#define INCH 0.0254
#define HOUR 3600
#define DENSITY 1000           //kg/m^3
#define VISCOSITY 0.0017       //Pa * s
#define ABSROUGHNESS 0.00001    //rugosidade absoluta em m
#define G 9.81
#define FPUMP 7
#define ELECBILL 0.75
#define IDLE 0.8
#define YEARS 5
#define SECINYEAR 31536000
#define JTOKWH 3600000
#define EFFEC 0.8
#define MAINTENANCE 0.015
#define K90 5
#define K180 8
#define KHEAT 22
#define KGLOBE 13

double reynolds         (double diameter, double velocity);
double frictionfactor   (double diameter, double reynolds);
double minorloss        (double constant, double diameter, double flow_rate);
double losspipe         (double friction, double diameter, double flow_rate, double length);

int main()
{
    int i, j;
    char pipenames[] = "1-23-1 A  B ";
    double flow_rate =  15;                             //m^3/h
    double diameter[9] = {0.5, 2, 3, 4, 6, 8, 10, 12, 16};   //in
    int length[4] =   {150, 200, 220, 10};            //comprimento em 1-2, 3-1, A e B
    double price[9] =    {0, 333, 443, 490, 805, 1050, 1210, 1407, 1863};  //preco por metro de tubulacao
    int elbow[4] =    {4, 4, 15, 10};                 //cotovelos
    int globe[4] =    {0, 0, 10, 2};                  //valv. globo totalmente abertas
    double initialcost[4][9];
    double pumpopercost[4][9];
    double headloss[4][9];
    double maintenancecost[4][9];
    double totalcost[4][9];
    int cheapest[4] = {3, 3, 3, 3};

    flow_rate /= HOUR;                                  //conversao m^3/h  => m^3/s
    for (i=0; i<9; i++)
    {
        diameter[i] *= INCH;    //converte todos os diametros de in para m. quando printar qualquer diametro, dividir por INCH
    }

    for (i=0; i<4; i++)
    {
        printf("-------------------\n-------------------\n\n");
        printf("Tubulacao %c%c%c, de %dm, com %d joelhos e %d valvulas globo (5 graus):\n\n",pipenames[3*i],pipenames[3*i+1],pipenames[3*i+2], length[i], elbow[i], globe[i]);
        for (j=1; j<9; j++)
        {
            printf("\t\tPara %2.1fin de diametro:\n", diameter[j]/INCH);
            /*k para joelho: estimado em 5
            k para valvula em globo: 13*/

            switch (i)
            {
            case 0:
                headloss[i][j] = losspipe(frictionfactor(diameter[j], flow_rate), diameter[j], flow_rate, 150)   +  4*minorloss(K90, diameter[j], flow_rate);
                break;          //perdas maiores;                                                                   4 joelhos 90 graus
            case 1:
                headloss[i][j] = losspipe(frictionfactor(diameter[j], flow_rate), diameter[j], flow_rate, 200)   +  4*minorloss(K90, diameter[j], flow_rate) + minorloss(KHEAT, diameter[j], flow_rate);
                break;          //perdas maiores                                                                    4 joelhos 90 graus                          trocador de calor
            case 2:
                headloss[i][j] = losspipe(frictionfactor(diameter[j], flow_rate/2), diameter[j], flow_rate, 220) + 15*minorloss(K90, diameter[j], flow_rate/2) + 10*minorloss(KGLOBE, diameter[j], flow_rate/2);
                                //perdas maiores                      (metade da vazao)                             15 joelhos 90 graus                          10 valvulas em globo
                printf("Perdas na tubulacao externa: \t\t\t%10.2f m^2/s^2\n", headloss[i][j]);
                headloss[i][j] += 10*minorloss(K180, diameter[0], flow_rate/2) + losspipe(frictionfactor(diameter[0], flow_rate/2), diameter[0], flow_rate/2, 10);
                break;          //perda por 10 joelhos 180                          //perda por tubulacao de 10m, 0.5"
            case 3:
                headloss[i][j] = losspipe(frictionfactor(diameter[j], flow_rate/2), diameter[j], flow_rate, 10)  + 10*minorloss(K90, diameter[j], flow_rate/2) +  2*minorloss(KGLOBE, diameter[j], flow_rate/2);
                                //perdas maiores                      (metade da vazao)                             15 joelhos 90 graus                           2 valvulas em globo
                printf("Perdas na tubulacao externa: \t\t\t%10.2f m^2/s^2\n", headloss[i][j]);
                headloss[i][j] += 10*minorloss(K180, diameter[0], flow_rate/2) + losspipe(frictionfactor(diameter[0], flow_rate/2), diameter[0], flow_rate/2, 10);
                break;          //perda por 10 joelhos 180                          //perda por tubulacao de 10m, 0.5"


            }
            printf("Perda de carga total: \t\t\t\t%10.2f m^2/s^2\n", headloss[i][j]);
            initialcost[i][j] = length[i]*price[j];                                                     // preco de tubulacao = preco/m * comprimento
            printf("Preco da tubulacao: \t\t\t\t%10.2f\n", initialcost[i][j]);
            printf("Preco da bomba para essa tubulacao (%dx): \t%10.2f\n",FPUMP, FPUMP*initialcost[i][j]);  //preco de equipamentos, etc

            initialcost[i][j] *= (1+FPUMP); //preco inicial total = preco bomba + preco tubulacao
            printf("Preco inicial total: \t\t\t\t%10.2f\n", initialcost[i][j]);

            /*ate aqui, o programa calcula o preco da tubulacao e bomba. a proxima parte se refere ao
            calculo da potencia por perda de pressao. a perda de carga eh dada em m^2/s^2, ou seja, e
            preciso converter pra W. essa conversao e dada multiplicando pela vazao massica, ou seja,
            densidade * vazao volumetrica.*/

            pumpopercost[i][j] = headloss[i][j]*DENSITY*flow_rate;  //esta em W
            pumpopercost[i][j] *= SECINYEAR*YEARS*IDLE;     //W * segundos * segundos em um ano * numero de anos * tempo de operacao (resultado: joule)
            pumpopercost[i][j] /= JTOKWH;       // 3.600.000 J = 1kWh
            pumpopercost[i][j] *= ELECBILL;     // preco do kWh
            pumpopercost[i][j] /= EFFEC;        // eficiencia da bomba
            printf("Preco de operacao da bomba (5 anos): \t\t%10.2f\n", pumpopercost[i][j]);

            maintenancecost[i][j] = MAINTENANCE*YEARS*initialcost[i][j];    //aqui nao considero tempo de ociosidade
            printf("Preco de manuntencao: \t\t\t\t%10.2f\n\n", maintenancecost[i][j]);

            totalcost[i][j] = initialcost[i][j]+pumpopercost[i][j]+maintenancecost[i][j];
            printf("Preco total : \t\t\t\t\t%10.2f\n\n", totalcost[i][j]);
        }
    }

    for (i=0; i<4; i++)
    {
        for (j=1; j<9; j++)
        {
            if (totalcost[i][j] < totalcost[i][cheapest[i]])
            {
                cheapest[i] = j;
            }
        }
        printf("Tubulacao mais barata na linha %c%c%c: %2.1fin\nCusto total:\t%10.2f\n",pipenames[3*i],pipenames[3*i+1],pipenames[3*i+2], diameter[cheapest[i]]/INCH, totalcost[i][cheapest[i]]);
    }

    return 0;
}

double reynolds (double diameter, double velocity) //calcula numero de reynolds pra alta temperatura
{
    double reynoldsnumber;

        reynoldsnumber = DENSITY*velocity*diameter/VISCOSITY;
        //printf("Numero de Reynolds:\t\t%-8.0f\n", reynoldsnumber);

    return reynoldsnumber;
}
double f(double x, double eD, double re)
{
    double y = 1/sqrt(x) + 2*log10(eD/3.7 + 2.51/(re*sqrt(x)));
    return y;
}
double dfdx(double x, double eD, double re)
{
    double y = -(251/(100*log(10)*re*(251/(100*re*sqrt(x)+10/37*eD)*pow(x, 1.5)))) - 1/(2*pow(x, 1.5));
    return y;
}
double frictionfactor(double diameter, double flow_rate)     //metodo de newton pra calcular f
{
    double relroughness, area, velocity, reynoldsnumber;
    double tol = 0.00005, x = 0.000001;
    int i = 0;

    area = PI*diameter*diameter/4;          // A = pi*d^2/4
    velocity = flow_rate/area;
    relroughness = ABSROUGHNESS/diameter;
    reynoldsnumber = reynolds(diameter, velocity);

    while (fabs(f(x, relroughness, reynoldsnumber)) > tol )
    {
        x -= (f(x, relroughness, reynoldsnumber)/dfdx(x, relroughness, reynoldsnumber));
        i++;
    }
    //printf("Fator de friccao aproximado:\t%1.6f\n",x);

    return x;
}
double minorloss (double constant, double diameter, double flow_rate)
{
    double area = PI*diameter*diameter/4;
    double velocity = flow_rate/area;
    double loss;

    loss = constant*velocity*velocity/2;
    return loss;
}
double losspipe (double friction, double diameter, double flow_rate, double length)
{
    double area = PI*diameter*diameter/4;
    double velocity = flow_rate/area;
    double loss;

    loss = friction*length*velocity*velocity/(2*diameter);  //hl = fV^2/
    return loss;                                            //      2D
}
