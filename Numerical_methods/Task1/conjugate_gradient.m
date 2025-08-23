function x = conjugate_gradient (A, b, delta, epsilon, max_iterations)
  x = zeros(size(b)); % x_0 (początkowe przybliżenie rozwiązania)

  r = b - A * x; % r = r0 (reszta)
  v = r; % v = v0 (kierunek  poszukiwań prostopadły do poprzednich kierunków)
  c = r' * r; % c = suma kwadratow reszty
  for k = 1:max_iterations
    if (v' * v < delta^2) printf("Warunek stopu - delta \n"); return endif % Jak bardzo można poprawic rozwiązanie
    z = A * v;
    t = c / (v' * z); % t = t_{k} % t to optymalna długosc kroku
    x = x + t * v; % x = x_{k+1}  % nowe rozwiązanie to poprzednie przsunięte w kierunku `v` o długosc `t`
    printf("Iteracja %d - Błąd: %d\n", k, norm(b - A * x, 2))
    r = r - t * z; % r = r_{k+1}
    d = r' * r; % suma kwadratow reszty
    if (d < epsilon^2) printf("Warunek stopu - epsilon \n"); return endif % Reszta mniejsza od epsilona
    v = r + (d/c) * v; % v = v_{k+1} (Nowy kierunek poszukiwań)
    c = d;
  endfor

endfunction
