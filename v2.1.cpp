#include <stdio.h>
#include <math.h>

#define VER 2.1
/*
    varios comentarios adicionados
    otimizacoes menores
    calculo de tubulacao com diametros variando entre 0 e 16 in adicionada
    switch is ded
    linhas de codigo relacionadas a debugs removidas
    display final de custos e diametros adicionados
*/

#define PI 3.14159265
#define INCH 0.0254             //1 polegada = 2,54 cm
#define HOUR 3600               //1 hora = 3600s
#define DENSITY 1000            //kg/m^3
#define VISCOSITY 0.0017        //Pa * s
#define ABSROUGHNESS 0.00001    //Rugosidade absoluta de tubo de aço trefilado em m
#define G 9.81                  //m/s^2
#define FPUMP 7                 //Fator de custo da bomba
#define ELECBILL 0.75           //Custo em R$/kWh
#define IDLE 0.8                //Ociosidade de 20%
#define YEARS 5                 //Duracao de 5 anos
#define SECINYEAR 31536000      //Numero de segundos em um ano
#define JTOKWH 3600000          //Numero de j em um kWh
#define EFFEC 0.8               //Eficiencia da bomba
#define MAINTENANCE 0.015       //1,5% de manutencao
#define K90 5                   //Valor arbitrario
#define K180 8                  //Valor arbitrario
#define KHEAT 22                //Do edital
#define KGLOBE 13               //Valor arbitrario

double reynolds         (double diameter, double velocity);                                     //Funcao que calcula o numero de reynolds
double frictionfactor   (double diameter, double reynolds);                                     //Fator de atrito a partir de reynolds e diametro
double minorloss        (double constant, double diameter, double flow_rate);                   //Perdas menores, usando um K
double losspipe         (double friction, double diameter, double flow_rate, double length);    //Perdas maiores, precisa do fator de atrito

int main()
{
    int i, j;
    char pipenames[] = "1-23-1 A  B ";                                      //Permite printar os nomes das tubulacoes
    int length[4] =        {150, 200, 220, 10};                             //Comprimento em 1-2, 3-1, A e B
    int internaltube[4] =  {0,     0,  10, 10};                             //Tubulacao interna em A e B
    int internalelbow[4] = {0,     0,  10, 10};                             //Joelhos em A e B
    int elbow[4] =         {4,     4,  15, 10};                             //Numero de cotovelos
    int globe[4] =         {0,     0,  10,  2};                             //Numero de valv. globo totalmente abertas
    int heatex[4] =        {0,     1,   0,  0};                             //Somente 3-1 tem trocador de calor

    double flow_rate =  15;                                                 //Vazao, em m^3/h
    double diameter[9] =   {0.5, 2,   3,   4,   6,    8,   10,   12,   16}; //in
    double price[9] =      {0, 333, 443, 490, 805, 1050, 1210, 1407, 1863}; //Preco por metro de tubulacao
    double initialcost[4][9];           //Matriz que vai conter os custos iniciais. i=tubulacao (A, B, 1-3...) j=diametro
    double pumpopercost[4][9];          //Matriz que vai conter custos de operacao da bomba (eletricidade)
    double headloss[4][9];              //Matriz que vai conter a perda de carga em (m/s)^2
    double maintenancecost[4][9];       //Matriz para custo de manutencao
    double totalcost[4][9];             //Matriz para somatorio de custos
    int cheapest[4] = {3, 3, 3, 3};     //Armazena qual diametro e o mais barato

    flow_rate /= HOUR;                  //Conversao m^3/h  => m^3/s
    for (i=0; i<9; i++)
    {
        diameter[i] *= INCH;            //Converte todos os diametros de in para m. Quando printar qualquer diametro, dividir por INCH
    }

    for (i=0; i<4; i++)
    {
        if (i==2 || i==3)
        {
            flow_rate /=2;              //Se for a tubulacao A ou B, considera metade da vazao
        }
        printf("---------------------------\n---------------------------\n\nTubulacao %c%c%c, de %dm, com %d joelhos e %d valvulas globo (5 graus):\n\n",pipenames[3*i],pipenames[3*i+1],pipenames[3*i+2], length[i], elbow[i], globe[i]);

        for (j=1; j<9; j++)
        {
                printf("\t\tPara %2.1fin de diametro:\n", diameter[j]/INCH);

            headloss[i][j] = losspipe(frictionfactor(diameter[j], flow_rate), diameter[j], flow_rate, length[i]);   //Perdas maiores: fLV^2/2
            headloss[i][j] += elbow[i] *minorloss(K90, diameter[j], flow_rate);                                     //Perdas devido a joelhos de 90 graus
            headloss[i][j] += heatex[i]*minorloss(KHEAT, diameter[j], flow_rate);                                   //Perdas no trocador de calor
            headloss[i][j] += globe[i] *minorloss(KGLOBE, diameter[i], flow_rate);                                  //Perdas nas valvulas globo

                printf("Perdas na tubulacao externa: \t\t\t%10.2f m^2/s^2\n", headloss[i][j]);

            headloss[i][j] += internaltube[i]*losspipe(frictionfactor(diameter[0], flow_rate), diameter[0], flow_rate, 10);
            //Perda de carga pela tubulacao interna dos equipamentos de ar condicionado
            headloss[i][j] += internalelbow[i]*minorloss(K180, diameter[0], flow_rate);



                printf("Perda de carga total: \t\t\t\t%10.2f m^2/s^2\n", headloss[i][j]);


            initialcost[i][j] = length[i]*price[j];         //Preco de tubulacao = preco/m * comprimento
                printf("Preco da tubulacao: \t\t\t\t%10.2f\n", initialcost[i][j]);
                printf("Preco da bomba para essa tubulacao (%dx): \t%10.2f\n",FPUMP, FPUMP*initialcost[i][j]);
            //Preco de equipamentos, etc = fator (=7) * custo da tubulacao


            initialcost[i][j] *= (1+FPUMP); //preco inicial total = preco bomba + preco tubulacao = (1 + fator) * custo da tubulacao
                printf("Preco inicial total: \t\t\t\t%10.2f\n", initialcost[i][j]);

            /*   Ate aqui,  o programa calcula o preco da tubulacao e bomba. Tambem ja obtemos a perda  de
            carga. A proxima parte se refere ao calculo do custo da energia devido a por perda de pressao.
                 A perda de carga eh dada em m^2/s^2, ou seja, e preciso converter pra W. Essa conversao e
            dada multiplicando pela vazao massica, ou seja, densidade * vazao volumetrica. Apos isso, pre-
            cisamos encontrar  o custo em kWh. Fazemos isso multiplicando a potencia em W  pelo numero  de
            segundos em cinco anos, levando em conta o tempo de ociosidade. Esse resuldado estara em J.
            Apos isso, basta converter de J para kWh.
                 Por fim, multiplicamos os kWh pelo seu custo em reais. Isso da o gasto total.             */

            pumpopercost[i][j] = headloss[i][j]*DENSITY*flow_rate;  //Esta em W
            pumpopercost[i][j] *= SECINYEAR*YEARS*IDLE;             //W * segundos * segundos em um ano * numero de anos * tempo de operacao (resultado: J)
            pumpopercost[i][j] /= JTOKWH;                           //3.600.000 J = 1kWh
            pumpopercost[i][j] *= ELECBILL;                         //Preco do kWh
            pumpopercost[i][j] /= EFFEC;                            //Eficiencia da bomba
                printf("Preco de operacao da bomba (5 anos): \t\t%10.2f\n", pumpopercost[i][j]);

            maintenancecost[i][j] = MAINTENANCE*YEARS*initialcost[i][j];
            /*Aqui nao considero tempo de ociosidade (manutencao ocorre estando ligada ou nao)
            Custo de manutencao = 1,5% * 5 anos * custo inicial                           */
                printf("Preco de manutencao: \t\t\t\t%10.2f\n\n", maintenancecost[i][j]);

            totalcost[i][j] = initialcost[i][j] + pumpopercost[i][j]+maintenancecost[i][j];
            //Custo total = custo inicial + custo da eletricidade + custo de manutencao

                printf("Preco total : \t\t\t\t\t%10.2f\n\n", totalcost[i][j]);
        }

    if (i==2 || i==3)
        {
            flow_rate *=2;              //Reverte a divisao apos finalizar os calculos
        }
    }


    for (i=0; i<4; i++)     /*Organizador de custos: se o custo do diametro X for menor que o do diametro Y, o o valor de X e assinalado
                            a Y. O programa continua analisando os proximos diametros para ver se eles sao mais baratos que o novo Y. */
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
    printf("\nLinha\t\tDiametro\t\tCusto total\n");
    for (i=0; i<4; i++)
    for (j=1; j<9; j++)
    {
        printf("%c%c%c\t\t%2.2f in \t\t%7.2f\n",pipenames[i], pipenames[i+1], pipenames[i+2], diameter[j]/INCH, totalcost[i][j]);
    }

    FILE *tabela;                   /*Faz uma aproximacao dos custos parciais com 100 pontos para uma tubulacao de 220m.
                                    Considera a mesma vazao do problema (15 m^3/h).
                                    A aproximacao e salva em um documento de texto que, quando copiado e colado no excel,
                                    pode ser usado para gerar uma tabela que gera o grafico utilizado no trabalho.
                                    Atraves de regressao linear descobriu-se que a melhor aproximacao para os custos
                                    da tubulacao e 111.7 + 110x (r^2 = 0.9955).*/
    tabela = fopen("tabela.txt", "w");
    fprintf(tabela, "Diametro(in)\tCusto inicial\n");
    double diamtable;
    for (i=0; i<100; i++)
    {
        diamtable = (i+1)*0.16;     //Diametro aumenta em incrementos de 16/100
        fprintf(tabela, "%-2.2f\t%-8.2f\n", diamtable, 220*(111.7+110*i)*(1+FPUMP));  //custo = (1 + fator)*220m*custo por metro
    }
    fprintf(tabela, "\n\nDiametro(in)\tCusto da perda de carga\n");
    for (i=0; i<100; i++)
    {
        diamtable = (i+1)*0.16;
        diamtable *=INCH;           //Abaixo, imprime na tabela o custo associado a perda de carga para cada diametro.
        fprintf(tabela, "%-2.2f\t%-8.2f\n", diamtable/INCH, DENSITY*flow_rate*SECINYEAR*YEARS*IDLE/JTOKWH*ELECBILL/EFFEC*losspipe(frictionfactor(diamtable, flow_rate), diamtable, flow_rate, 220));
    }
    fprintf(tabela, "\n\nDiametro(in)\tCusto da manutencao\n");
    for (i=0; i<100; i++)
    {
        diamtable = (i+1)*0.16;     //Imprime na tabela o custo da manutencao.
        fprintf(tabela, "%-2.2f\t%-8.2f\n", diamtable, MAINTENANCE*YEARS*220*(111.7+110*i)*(1+FPUMP));
    }
    fclose(tabela);

    return 0;
}

double reynolds (double diameter, double velocity)          //calcula numero de reynolds
{
    double reynoldsnumber;
    reynoldsnumber = DENSITY*velocity*diameter/VISCOSITY;   //Re = pVD/u
    return reynoldsnumber;
}
double f(double x, double eD, double re)                    //Equacao de colebrook. necessaria para metodo de newton.
{
    double y = 1/sqrt(x) + 2*log10(eD/3.7 + 2.51/(re*sqrt(x)));
    return y;
}
double dfdx(double x, double eD, double re)                 //Derivada parcial da equacao de colebrook em relacao a f. necessaria para metodo de newton
{
    double y = -(251/(100*log(10)*re*(251/(100*re*sqrt(x)+10/37*eD)*pow(x, 1.5)))) - 1/(2*pow(x, 1.5));
    return y;
}
double frictionfactor(double diameter, double flow_rate)    //Metodo de newton pra calcular f
{
    double relroughness, area, velocity, reynoldsnumber;
    double tol = 0.00005, x = 0.000001;                     //Tolerancia da resposta para f bastante baixa. chute inicial definido.
    int i;

    area = PI*diameter*diameter/4;                  //A = pi*d^2/4
    velocity = flow_rate/area;                      //v = V/A
    relroughness = ABSROUGHNESS/diameter;           //Rugosidade relativa = e/D
    reynoldsnumber = reynolds(diameter, velocity);  //Reynolds do escoamento

    for (i = 0; fabs(f(x, relroughness, reynoldsnumber)) > tol; i++)        //Magia do metodo de newton. Codigo ancestral, nao toque
    {
        x -= (f(x, relroughness, reynoldsnumber)/dfdx(x, relroughness, reynoldsnumber));
    }
    return x;                                       //Retorna o fator de friccao.
}
double minorloss (double constant, double diameter, double flow_rate)
{
    double area = PI*diameter*diameter/4;       //A = pi*d^2/4
    double velocity = flow_rate/area;           //v = V/A
    double loss;

    loss = constant*velocity*velocity/2;        //hl = kv^2/2
    return loss;
}
double losspipe (double friction, double diameter, double flow_rate, double length)
{
    double area = PI*diameter*diameter/4;       //A = pi*d^2/4
    double velocity = flow_rate/area;           //v = V/A
    double loss;

    loss = friction*length*velocity*velocity/(2*diameter);  //hl = fV^2/2D
    return loss;
}
