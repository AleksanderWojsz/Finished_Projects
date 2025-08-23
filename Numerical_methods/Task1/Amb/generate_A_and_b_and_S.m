function [A, b, S] = generate_A_and_b_and_S (N, c, f)
  h = 1 / N;

  % Tworzenie macierzy T
  neg_ones_vector = ones(N-2, 1) *-1; % Wektor `-1`
  bs_vector = ones(N-1, 1) * (4+h^2*c); % Wektor `b`
  T = sparse(diag(neg_ones_vector, -1) + diag(neg_ones_vector, 1) + diag(bs_vector));

  % Tworzenie macierzy A
  I = sparse(eye(N-1));
  A = sparse(kron(diag(neg_ones_vector, -1) + diag(neg_ones_vector, 1), I) + kron(I, T)); % https://en.wikipedia.org/wiki/Kronecker_product
  A /= h^2;

  % Tworzenie b
  [xx, yy] = meshgrid(1:N-1, 1:N-1);
  b = f(xx/N, yy/N);
  b = reshape(b', (N-1)^2, 1); % Transpozycja, bo domyślnie nakłada się kolumny

  % Tworzenie S
  S = sparse(kron(I, T));



endfunction
