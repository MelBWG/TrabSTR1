data = csvread('D:\STR\TrabSTR1\hello_world\lcd_driver_wcrt.dat');
% Pega colunas do arquivo csv
a = data(:,1); 
b = data(:,2); 
c = data(:,3);
d = data(:,4);

% Calcula tempo da parte inicial da task
x = b-a;
disp(x);
% Calcula tempo do loop while da task
y = d-c;
disp(y);

% Calcula tempo total da task na primeira execução
z=x+y;
z=z/1000;
disp(z)
% Calcula média e dp das medições
S=std(z);
M=mean(z);
disp(M);
disp(S);
% Mostra histograma das medidas com normal
H= histfit(z);
hold on;
title("WCRT Estimado para a Task do Motor");
xlabel("Tempo de Resposta (ms)");
ylabel("Quantidade de Medidas");
legend("Medições","Normal equivalente");