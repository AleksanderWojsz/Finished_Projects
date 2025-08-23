% epsilon - dokładność przybliżenia
function x = preconditioning_conjugate_gradient (A, b, S, epsilon, max_iterations)
  x = zeros(size(b)); % x_0

  r = b - A * x; % r = r0
  z = linsolve(S, r, struct("SYM", true)); % rozwiązywanie układu Sz = r
  v = z; % v = v0
  c = z' * r;
  for k = 1:max_iterations
    z = A * v;
    t = c / (v' * z); % t = t_{k}
    x = x + t * v; % x = x_{k+1}
    printf("Iteracja %d - Błąd: %d\n", k, norm(b - A * x, 2))
    r = r - t * z; % r = r_{k+1}
    z = linsolve(S, r, struct("SYM", true)); % rozwiązywanie układu Sz = r
    d = z' * r;

    if (d < epsilon^2)
      if (r' * r < epsilon^2)
        printf("Warunek stopu\n");
        return
      endif
    endif

    v = z + (d/c) * v; % v = v_{k+1}
    c = d;
  endfor

endfunction
