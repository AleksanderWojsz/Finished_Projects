#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <climits>
#include <set>

using namespace std;

class Node {
public:
    int key;
    int count;
    int size_of_left_subtree; // bez obecnego wierzcholka
    Node *left;
    Node *right;
    int height;

    Node(int key)
            : key(key), count(1), height(1), left(NULL), right(NULL), size_of_left_subtree(0) {}
};

int get_height(Node* root) {
    return root != NULL ? root->height : 0;
}

int get_balance_difference(Node* root) {
    return get_height(root->left) - get_height(root->right);
}

void update_height(Node* root) {
    root->height = max(get_height(root->left), get_height(root->right)) + 1;
}

/*
 A
  \      rotacja w lewo
   B        ------->     B
    \                   /  \
     C                 A    C
 */
Node* rotate_left(Node* a) {
    Node* b = a->right;
    Node* b_left = b->left;

    b->left = a;
    a->right = b_left;

    update_height(a);
    update_height(b);

    b->size_of_left_subtree += a->size_of_left_subtree + a->count;

    return b;
}

Node* rotate_right(Node* a) {
    Node* b = a->left;
    Node* b_right = b->right;

    b->right = a;
    a->left = b_right;

    update_height(a);
    update_height(b);

    a->size_of_left_subtree -= b->size_of_left_subtree + b->count;

    return b;
}

Node* add_increasing(Node* root, int key) {


    if (root == NULL) { // trafilismy na koniec
        return new Node(key);
    }
    else if (key < root->key) { // idziemy w lewo
        root->size_of_left_subtree++;
        root->left = add_increasing(root->left, key);
    }
    else if (key > root->key) { // idziemy w prawo
        root->right = add_increasing(root->right, key);
    }
    else if (key == root->key) { // jestesmy w srodku
        root->count++;
    }

    // teraz bedziemy sie cofali po sciezce aktualizujac wysokosc i balansujac
    update_height(root);

    if (get_balance_difference(root) > 1) { // po lewej jest wiecej
        if (root->left->key < key) {
            root->left = rotate_left(root->left);
        }
        return rotate_right(root);
    }
    else if (get_balance_difference(root) < -1) { // po prawej jest wiecej
        if (root->right->key > key) {
            root->right = rotate_right(root->right);
        }
        return rotate_left(root);
    }

    return root;
}



int number_of_elements_less_than(Node* root, int key) {
    if (root == NULL) {
        return 0;
    }
    else if (key < root->key) {
        return number_of_elements_less_than(root->left, key);
    }
    else if (key > root->key) {
        return root->size_of_left_subtree + number_of_elements_less_than(root->right, key) + root->count;
    }
    else if (key == root->key) {
        return root->size_of_left_subtree;
    }
}

Node* remove(Node* root, int key, int count_to_remove) {

    if (root == NULL) {
        return NULL;
    }
    else if (key < root->key) { // idziemy w lewo
        root->size_of_left_subtree -= count_to_remove;
        root->left = remove(root->left, key, count_to_remove);
    }
    else if (key > root->key) { // idziemy w prawo
        root->right = remove(root->right, key, count_to_remove);
    }
    else { // jestesmy w node do usuniecia
        if (root->count > 1) {
            root->count--;
            return root;
        }
        else if (root->left == NULL || root->right == NULL) {
            Node* to_delete;

            if (root->left == NULL && root->right == NULL) { // nie ma dzieci
                to_delete = root;
                root = NULL;
            }
            else if (root->left == NULL) { // ma tylko prawe dziecko
                to_delete = root->right;
                *root = *to_delete;
            }
            else { // ma tylko lewe dziecko
                to_delete = root->left;
                *root = *to_delete;
            }

            delete to_delete;
        }
        else { // ma dwojke dzieci
            Node* temp = root->right;
            while (temp->left != NULL) {
                temp = temp->left;
            }

            // zastepujemy node'a do usuniecia nodem temp
            root->key = temp->key;
            root->count = temp->count;

            // trzeba zaktualizowac sciezke z ktorej wzielismy temp bo mogl byc count np 2

            temp->count = 1; // zeby dalo sie usunac temp
            root->right = remove(root->right, temp->key, root->count);
        }
    }

    if (root == NULL) {
        return NULL;
    }
    else {
        update_height(root);
    }

    int weight = get_balance_difference(root);

    if (weight > 1) {
        if (get_balance_difference(root->left) < 0) {
            root->left = rotate_left(root->left);
        }

        return rotate_right(root);
    }
    else if (weight < -1) {
        if (get_balance_difference(root->right) > 0) {
            root->right = rotate_right(root->right);
        }

        return rotate_left(root);
    }

    return root;
}


int main() {

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    int n, k, a, b, t;
    std::cin >> n >> k;

    vector<tuple<int, int, int>> zapytania;
    zapytania.reserve(n);
    for (int i = 0; i < n; i++) {
        std::cin >> a >> b >> t;
        zapytania.emplace_back(a, b, t);
    }

    // sortujemy zapytania po czasie rosnoaco
    sort(zapytania.begin(), zapytania.end(), [](const tuple<int, int, int>& a, const tuple<int, int, int>& b) {
        return get<2>(a) < get<2>(b);
    });



    Node* konce = NULL;
    Node* poczatki = NULL; // wartosci wstawiac na minusie zeby byly posortowane malejaco
    int poczatkowy_t_index = 0;
    long long unsigned int result = 0;

    int liczba_przedzialow = 1;
    konce = add_increasing(konce, get<1>(zapytania[0]));
    poczatki = add_increasing(poczatki, -get<0>(zapytania[0]));
    for (int i = 1; i < n; i++) {
        a = get<0>(zapytania[i]);
        b = get<1>(zapytania[i]);
        t = get<2>(zapytania[i]);

        // przesuwamy przedzial czasu
        while (t - get<2>(zapytania[poczatkowy_t_index]) > k) {

            // usuwam poczatek i koniec
            konce = remove(konce, get<1>(zapytania[poczatkowy_t_index]), 1);
            poczatki = remove(poczatki, -get<0>(zapytania[poczatkowy_t_index]), 1);
            liczba_przedzialow--;
            poczatkowy_t_index++;
        }

        // licze kolizje
        result += liczba_przedzialow - number_of_elements_less_than(konce, a) - number_of_elements_less_than(poczatki, -b);

        // dodaje poczatek i koniec
        konce = add_increasing(konce, b);
        poczatki = add_increasing(poczatki, -a);
        liczba_przedzialow++;

    }

    cout << result << "\n";

    return 0;
}

/*
Sortujemy zapytania po czasie rosnąco.
Następnie przechodzimy po tych zapytaniach i trzymamy w
drzewie avl czasy początków i końców przedzialów poprzedzających aktualny przedział i których czas jest 
różny o co najwyżej k względem aktualnego.
 
 Jak któryś przedział przestaje być aktualny to usuwamy jego poczatek i koniec z drzew.
 
 Wynik dla każdego przedziału liczymy odejmując od wszystkich przedziałów 
 te które zaczynają się za wcześnie lub kończą za późno.
 */