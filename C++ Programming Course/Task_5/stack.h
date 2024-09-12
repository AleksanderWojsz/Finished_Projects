#ifndef CPP_5_STACK_H
#define CPP_5_STACK_H

#include <list>
#include <map>
#include <memory>
#include <stdexcept>

namespace cxx {

    template <typename K, typename V>
    class stack {
        using pair_t = std::pair<V, size_t>;
        using pointer_t = typename std::map<K, std::list<pair_t>>::iterator;

        class Data { // Wszystkie dane obiektu.
        public:
            size_t stack_size = 0;
            size_t element_id = 0; // Każdy kolejny dodany element ma większy numer.
            std::map<K, std::list<pair_t>> values; // drzewo<klucz, lista<wartość, nr_elementu>>. Nowe elementy dodajemy na koniec listy; najnowsze będą na jej początku.
            std::map<size_t, pointer_t> stack_order;  // drzewo<nr_elementu, iterator do values>
            bool has_external_reference = false; // Jeśli do danych istnieje chociaż jedna referencja, to już zawsze będą kopiowane
        };

        std::shared_ptr<Data> data; // Wskaźnik na własne lub cudze dane.

        void detach() { // Zamienia cudze dane na własną kopię (chyba że istnieje referencja do obiektu, wtedy nie chcemy tworzyć).
            if (!data.unique() && !data->has_external_reference) {
                copy_from(data);
            }
        }

        void copy_from(std::shared_ptr<Data> source) {
            data = std::make_shared<Data>(*source);
            data->has_external_reference = false;

            // Aktualizacja iteratorów w stack_order.
            for (auto& pair : data->stack_order) {
                auto key = pair.second->first;
                pair.second = data->values.find(key); // Szukamy nowego iteratora dla tego samego klucza w skopiowanej mapie 'values'.
            }
        }

        void check_for_empty_stack() const {
            if (size() == 0) {
                throw std::invalid_argument("Exception: Empty stack");
            }
        }

        void check_for_key_existence(const K& key) const {
            if (!data->values.contains(key)) {
                throw std::invalid_argument("Exception: No such key");
            }
        }

    public:

        // konstruktor domyślny.
        stack() : data(std::make_shared<Data>()) {}

        // Konstruktor kopiujący.
        stack(const stack& other) {
            if (other.data->has_external_reference) {
                copy_from(other.data);
            } else {
                data = other.data; // Współdzieli dane.
            }
        }

        // Konstruktor przenoszący.
        stack(stack&& other) noexcept
                : data(std::move(other.data)){ // other.data to lvalue, bo to składowa obiektu, wiec trzeba zamienić na rvalue.
            other.data = nullptr;
        }

        stack& operator=(stack other) {
            data = other.data;
            return *this;
        }

        void push(const K& key, const V& value) {
            detach();
            data->has_external_reference = false; // Zmodyfikowaliśmy stos, wiec referencje do niego stały się nieważne.

            // Dodanie do drzewa z kluczami.
            if (data->values.contains(key)) {
                data->values.at(key).push_back(std::make_pair(value, data->element_id));
            } else {
                data->values.insert({key, std::list<pair_t>{std::make_pair(value, data->element_id)}});
            }

            // Dodanie do drzewa kolejności.
            data->stack_order.insert({data->element_id, data->values.find(key)}); // 'find' zwraca iterator

            data->element_id++;
            data->stack_size++;
        }

        void pop() { // Ten pop nie robi detach, bo robi to pop(key) którego wywołuje.
            check_for_empty_stack();

            // Znajdujemy i usuwamy najwcześniej dodany element ze 'stack_order'.
            K first_on_stack = data->stack_order.rbegin()->second->first;
            pop(first_on_stack);
        }

        void pop(const K& key) {

            check_for_empty_stack();
            check_for_key_existence(key);
            detach();
            data->has_external_reference = false;

            size_t deleted_id = data->values.at(key).back().second;
            data->values.at(key).pop_back();

            // Jak kolejka zrobiła się pusta, to usuwamy node'a.
            if (data->values.at(key).empty()) {
                data->values.erase(key);
            }

            data->stack_order.erase(deleted_id);
            data->stack_size--;
        }

        std::pair<K const &, V &> front() {

            check_for_empty_stack();
            detach();
            data->has_external_reference = true;

            K const &key = data->stack_order.rbegin()->second->first;
            V &value = data->values.at(key).back().first;
            return {key, value};
        }

        std::pair<K const &, V const &> front() const {
            check_for_empty_stack();

            K const &key = data->stack_order.rbegin()->second->first;
            V const &value = data->values.at(key).back().first;
            return {key, value};
        }

        V & front(K const &key) {

            check_for_empty_stack();
            check_for_key_existence(key);
            detach();
            data->has_external_reference = true;

            return data->values.at(key).back().first;
        }

        V const & front(const K &key) const {
            check_for_empty_stack();
            check_for_key_existence(key);

            return data->values.at(key).back().first;
        }

        size_t size() const {
            return data->stack_size;
        }

        size_t count(const K& key) const {
            return data->values.contains(key) ? data->values.at(key).size() : 0;
        }

        void clear() {

            // Jak nikt nie używa naszych danych, to je czyścimy.
            if (data.unique()) {
                data->values.clear();
                data->stack_order.clear();
                data->stack_size = 0;
                data->element_id = 0;
                data->has_external_reference = false;
            } // Jak ktoś używa, to tworzymy sobie nowe.
            else {
                data = std::make_shared<Data>();
            }
        }

        class const_iterator {
        private:
            using map_iter_t = typename std::map<K, std::list<pair_t>>::const_iterator;
            map_iter_t iter;

        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = const K;
            using pointer           = const K*;
            using reference         = const K&;

            const_iterator() : iter() {}

            explicit const_iterator(map_iter_t it) : iter(it) {}

            reference operator*() const {
                return iter->first;
            }

            pointer operator->() const {
                return &(iter->first);
            }

            const_iterator& operator++() {
                ++iter;
                return *this;
            }

            const_iterator operator++(int) {
                const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator== (const const_iterator& a, const const_iterator& b) {
                return a.iter == b.iter;
            }

            friend bool operator!= (const const_iterator& a, const const_iterator& b) {
                return a.iter != b.iter;
            }
        };

        const_iterator cbegin() const {
            return const_iterator(data->values.cbegin());
        }

        const_iterator cend() const {
            return const_iterator(data->values.cend());
        }

    };

} // namespace cxx

#endif //CPP_5_STACK_H
