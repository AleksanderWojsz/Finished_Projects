clear;
clear -global;

a = -5; % Początek przedziału
b = 5; % Koniec przedziału
c = 5; % Skalowanie liczby próbek (punktów na wykresie)
N = 15; % Liczba funkcji B-sklejanych
n = 3; % Stopień funkcji B-sklejanej

% Funkcje z polecenia
function retval = f(x)
  % retval = 1 / (x^2 + 1);
  retval = 1 / ((x + 2)^2 + 1) + 1 / ((x - 2)^2 + 1);
endfunction






h = (b - a) / (N - 2*n);
global us = a + ((0:N) - n) * h;


function retval = get_B_spline(x, n, i)
  global us;
  if (n == 0)

    if (x >= us(i) && x < us(i+1))
      retval = 1;
    else
      retval = 0;
    endif

  else
    retval = (x - us(i)) / (us(i+n) - us(i)) * get_B_spline(x, n-1, i) + (us(i+n+1) - x) / (us(i+n+1)-us(i+1)) * get_B_spline(x, n-1, i+1);
  endif
endfunction


M = c * (N - 2*n); % Liczba próbek funkcji f
h_hat = (b - a) / M; % Odległości między próbkami
xs = a + h_hat * (0:M);
xs = xs';
ys = arrayfun(@f, xs);

% Tworzenia macierzy A
A = sparse(zeros(M+1, N-n));
for j = 1:M+1
  for i = 1:N-n
    A(j, i) = get_B_spline(xs(j), n, i);
  endfor
endfor

% LZNK
d = A \ ys;

% Obliczenie s(x)
s_x = zeros(M+1, 1);
for i = 1:N-n
  s_x += d(i) * arrayfun(@(x) get_B_spline(x, n, i), xs);
endfor







% Wykresy
subplot (2, 1, 1)
plot(xs, ys, "o-", 'LineWidth', 1, xs, s_x, "o-", 'LineWidth', 1);
legend('Oryginał f(x)', 'Aproksymacja s(x)');
set(gca, 'fontsize', 16);
axis("tight");

subplot (2, 1, 2)
plot(xs, ys - s_x, "o-", 'LineWidth', 1)
title("Błąd f(x)-s(x)", 'FontSize', 16)
axis("tight");


