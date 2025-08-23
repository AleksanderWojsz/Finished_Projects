clear;
clear -global;
clc;

global a = 0; % Przedział dla t
global b = 15;
global N = 20; % Liczba węzłów-1 (knots) (N>2n)
global n = 4; % Stopień funkcji B-sklejanej
epsilon = 1e-6;

global d = [(linspace(0, 10, N-n))', (linspace(0, 0, N-n))']; % Współrzędne punktów kontrolnych (x, y)
% global d = [(linspace(0, 5, N-n))', (linspace(0, 5, N-n))']; % 4.09 * sqrt(2)

theta = linspace(0, 2*pi, N-n)';
radius = 5;
center = [5, 5];
% global d = [center(1) + radius * cos(theta), center(2) + radius * sin(theta)]; % 2 * pi * 5 = 31.4

global h = (b - a) / (N - 2*n);
global u = a + ((0:N) - n) * h; % Węzły. u_n = a, u_{N−n} = b


function retval = get_B_spline(n, i, t)
  global u;
  if (n == 0)

    if (t >= u(i) && t < u(i+1))
      retval = 1;
    else
      retval = 0;
    endif

  else
    retval = (t - u(i)) / (u(i+n) - u(i)) * get_B_spline(n-1, i, t) + (u(i+n+1) - t) / (u(i+n+1)-u(i+1)) * get_B_spline(n-1, i+1, t);
  endif
endfunction



% Pochodna s
function retval = s_derivative(t)
  global u;
  global d;
  global n;
  global N;

  retval = zeros(1, size(d)(2));
  for i=1:N-n-1 % Jest N-n-1 funkcji B-sklejanych (bo N-(n+2)+1)
    retval += n / (u(i+n+1)-u(i+1)) * (d(i+1, :) - d(i, :)) * get_B_spline(n-1, i+1, t);
  endfor
endfunction

% Norma z pochodnej s
function retval = s_derivative_norm(t)
  retval = norm(s_derivative(t), 2);
endfunction

% Metoda złożonej kwadratury Simpsona
function retval = simpson_integral(f, a, b, invervals)
  h = (b-a)/invervals;
  x = a + [0:2*invervals] * h / 2; %  x należy do [a, b], ale kroki są dwa razy częstsze

  summation = 0;
  for i = 2:invervals
    summation += 2*f(x(2*i-1)) + 4*f(x(2*i));
  endfor

  retval = h / 6 * (f(x(1)) + 4*f(x(2)) + summation + f(x(2*invervals+1)));
endfunction


error = 1e6;
intervals = 2;
while error >= 15 * epsilon
  intervals *= 2;
  result1 = simpson_integral(@s_derivative_norm, a, b, intervals);
  result2 = simpson_integral(@s_derivative_norm, a, b, intervals / 2);
  error = abs(result1 - result2);
end

printf("Długość krzywej: %f. Liczba przedziałów %d\n", result1, intervals);






function draw_plot()
  global u;
  global d;
  global n;
  global a;
  global b;
  global N;
  t_values = linspace(a, b, 100);
  curve = zeros(length(t_values), size(d, 2));

  % Wartości krzywej B-sklejanej w punktach t
  for j = 1:length(t_values) # Dla każdego t
    t = t_values(j);
    curve(j, :) = zeros(1, size(d, 2));
    for i = 1:N-n # Sumujemy wszystkie B-funkcje
      curve(j, :) += d(i, :) * get_B_spline(n, i, t);
    end
  end



  plot(curve(:, 1), curve(:, 2), 'b-', 'LineWidth', 2); % Krzywa B-sklejana
  hold on;
  % plot(d(:, 1), d(:, 2), 'ro-', 'MarkerFaceColor', 'r'); % Punkty kontrolne
  xlabel('X');
  ylabel('Y');
  grid on;

endfunction

draw_plot();

