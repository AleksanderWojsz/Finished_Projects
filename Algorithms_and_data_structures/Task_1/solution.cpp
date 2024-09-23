/*
 ASD - Zadanie: MOZ
 Aleksander Wojsz aw450252
*/

#include<iostream>
#define MOD 1000000000
using namespace std;

int main() {

    int n, k;
    cin >> n >> k;
    k++; // Żeby k oznaczało maksymalną liczbę detali kafelka.

    int* zapytania = new int[n];
    for (int i = 0; i < n; i++) {
        cin >> zapytania[i];
    }

    auto* suma_wiersza = new long int[n];
    for (int i = 0; i < n; i++) {
        suma_wiersza[i] = 0;
    }
    suma_wiersza[0] = k;

    auto **tab = new long int*[2];
    for(int i = 0; i < 2; ++i) {
        tab[i] = new long int[k];
    }
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < k; j++) {
            tab[i][j] = 0;
        }
    }
    for (int i = 0; i < k; i++) {
        tab[0][i] = 1;
    }

    for (int i = 1; i < n; i++) {
        for (int j = 1; j < k - 1; j++) {
            tab[i % 2][j] = (tab[i % 2][j] + tab[(i - 1) % 2][j - 1]) % MOD;
            tab[i % 2][j] = (tab[i % 2][j] + tab[(i - 1) % 2][j]) % MOD;
            tab[i % 2][j] = (tab[i % 2][j] + tab[(i - 1) % 2][j + 1]) % MOD;

            suma_wiersza[i] = (suma_wiersza[i] + tab[i % 2][j]) % MOD;
        }

        tab[i % 2][0] = (tab[i % 2][0] + tab[(i - 1) % 2][0]) % MOD;
        tab[i % 2][0] = (tab[i % 2][0] + tab[(i - 1) % 2][1]) % MOD;
        tab[i % 2][k - 1] = (tab[i % 2][k - 1] + tab[(i - 1) % 2][k - 1]) % MOD;
        tab[i % 2][k - 1] = (tab[i % 2][k - 1]  + tab[(i - 1) % 2][k - 2]) % MOD;

        suma_wiersza[i] = (suma_wiersza[i] + tab[i % 2][0]) % MOD;
        suma_wiersza[i] = (suma_wiersza[i] + tab[i % 2][k - 1]) % MOD;

        for (int m = 0; m < k; m++) { // Zwolnienie niepotrzebnej już kolumny.
            tab[(i + 1) % 2][m] = 0;
        }
    }

    for (int i = 0; i < n; i++) {
        cout << suma_wiersza[zapytania[i] - 1] <<  " ";
    }
    
    // Złożoność pamięciowa - O(n + k)
    // Złożoność czasowa - O(n * k)

    return 0;
}