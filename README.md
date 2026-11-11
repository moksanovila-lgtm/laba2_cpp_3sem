# laba2_cpp_3sem

Задача: написать программу, которая строит индекс по колекции объектов (Person) - то есть ассоциативный массив "значение атрибута - список объектов с этим атрибутом". Индекс можно построить двумя разными структурами (BST, хеш-таблица). Нужно замерить время построения и поиска для каждой структуры на одних и тех же данных, сравнить, выгрузить в CSV, покрыть тестами, дать графический UI.
Это задача И-5 "Индексирование данных" + M-5/M-5.1(Разрешение коллизий с помощью списков)/M-5.2(Разрешение коллизий с помощью смещения адресов)

Сущности:

Предметная область:
Объект — Person
Класс: приватные поля firstName, middleName, lastName, birthYear; доступ через методы:
GetFirstName(), GetMiddleName(), GetLastName();
GetFullName() — «Фамилия Имя Отчество»;
GetFIO() — «Фамилия И.О.»;
GetBirthYear(), GetAge(int year).
Индекс строится по методам. ExtractKey(p, "fullName") -> p.GetFullName().

Атрибут — это какое свойство человека используется как ключ 

Простые: "fullName", "fio", "lastName", "birthYear".

Составные: "(lastName,birthYear,firstName)" 

Составной ключ — Sequence<char> (или std::string, если разрешено), склеенный через #. Тип ключа — единый для всех трёх структур.

Ключевое решение: уникальный ключ
Индекс строится по уникальному ключу: FullName или составной (LastName, BirthYear, FirstName). Дубликатов нет - все три структуры ведут себя одинаково при Add - сравнение честное.

Value в индексе — int (индекс Person в коллекции). Get(key) - int, по нему извлекается Person из коллекции. PersonList не используется — ключ уникален.

Вход и выход:
Sequence<Person> — из CSV (CsvLoader).
Список атрибутов — Sequence<std::string> (имена методов Person).
Выход: набор IDictionary<std::string, int> — индексов, по одному на атрибут. Доступ — PersonIndexer::GetIndex(attribute).

Ключ — это значение атрибута человека, например, "Иванов Иван Иванович" (полное ФИО).
Значение — это где этот человек лежит в вашей коллекции. Не сам человек, а номер (позиция) — 42. По номеру 42 вы достанете Person из коллекции.
Индекс — это структура, которая хранит пары «ключ -> номер». Например, BST, хеш-таблица.
Зачем индекс? Без него, чтобы найти «Иванова», надо перебрать всех 50 000 человек (O(n)). С индексом — один-два шага (O(1) или O(log n)).


Абстракции структур данных:
1. IDictionary<TKey, TElement> - интерфейс ассоциативного массива из методички М-5: GetCount, GetCapacity, Get, ContainsKey, Add, Remove  (IDictionary.h) 

2. HashIndexChain<K, V> - хеш-таблица: массив бакетов + цепочки. M-5.1 (HashIndexChain.h)
HashIndexChain опирается на ArraySequence<Bucket> (массив бакетов) и ListSequence<pair<K,V>> (цепочка)

3. HashIndexOpen<K, V> - хеш-таблица с открытой адресацией. Внутри: ArraySequence<Slot>, где Slot = {pair<K,V>, state} (HashIndexOpen.h)

4. TreeIndex<K, V> - обёртка над BinaryTree из ЛР-1, реализующая IDictionary (TreeIndex.h)

5. BinaryTree<K, V> - моё BST из ЛР-1 (сабмодуль). Оборачивается в TreeIndex (BinaryTree.h)



6. ICollection<T> - базовый интерфейс из ЛР-2 прошлого семестра(сабмодуль): Get(index), GetCount() (Collection.hpp)

7. IEnumerator<T> - интерфейс итератора из ЛР-2 прошлого семестра(сабмодуль): MoveNext, Current, Reset (IEnumerator.hpp)

8. Sequence<T> - Абстрактная последовательность из ЛР-2 прошлого семестра(сабмодуль): Get, Append, Prepend, InsertAt, Concat, Map, Where, Reduce, GetSubsequence (Sequence.hpp) 

9. ArraySequence<T> - Sequence на массиве DynamicArray (сабмодуль) (ArraySequence.hpp)

10. ListSequence<T> - Sequence на связном списке LinkedList (сабмодуль) (ListSequence.hpp)


Связи:
1. PersonIndexer - строит индекс по Person для заданного атрибута и заданной стратегии ("bst", "hash_chain", "hash_open") (PersonIndexer.h)

2. Benchmark - прогоняет 3 стратегии, замеряет время построения и среднее время поиска, пишет CSV.

3. графический UI

MainWindow (Qt) ──► CsvLoader ──► Sequence<Person>
 │                       |
 │                       │  data/persons_1000.csv
 │                       │  data/persons_5000.csv
 │                       │  ...
 │                       │
 ├── PersonIndexer ── IDictionary*
 │                         |── TreeIndex ──► BinaryTree (ЛР-1)
 │                         |── HashIndexChain
 │                         |── HashIndexOpen
 │
 └── Benchmark ── CsvLoader (для каждого n)
                ── PersonIndexer (для каждой стратегии)
                |── results/results.csv






Нефункциональные требования:
1. HashIndexChain::Get и HashIndexOpen::Get — O(1) в среднем
2. TreeIndex::Get — O(log n) на случайных, O(n) на отсортированных (BST без балансировки )
3. Get на отсутствующем ключе — std::out_of_range
4. Все три стратегии дают одинаковый Get на одних данных (тест эквивалентности)

Функциональные требования:
1. Загружать Sequence<Person> из CSV-файла заданного пути (CsvLoader::Load).
2. Сохранять Sequence<Person> в CSV-файл (CsvLoader::Save).
3. Обрабатывать ошибки CSV: неверное число полей, нечисловой birthYear, пустой файл — с понятным сообщением.
4. Выбирать стратегию индексирования: "bst", "hash_chain", "hash_open".
5. HashIndexOpen реализует IDictionary с разрешением коллизий открытой адресацией (M-5.2), внутри — ArraySequence<Slot>, Slot = {pair<string,int>, state}
6. Все три реализации доступны через IDictionary<string,int>* — PersonIndexer не знает, какая структура внутри.
7. HashIndexChain и HashIndexOpen автоматически перестраиваются: расширение при n = capacity, сжатие при n ≤ capacity/p (p ≥ q > 1).
8. PersonIndexer::GetIndex(attribute) возвращает указатель на индекс (или nullptr, если атрибут не индексировался).
9. Искать по ключу: IDictionary::Get(key) возвращает int — индекс Person в исходной коллекции.
10.	Проверять наличие ключа: IDictionary::ContainsKey(key).
11. При отсутствии ключа Get и Remove бросают std::out_of_range (п. 8).
12.	В UI: поле ввода ключа, выбор атрибута, кнопка «Найти» — результат (найденный Person или «не найдено») отображается в таблице.
13.	Все три стратегии на одних данных дают одинаковый результат Get (проверяется тестом эквивалентности).
14.	Замерять время построения индекса (PersonIndexer::Build) для каждой стратегии × каждого n × каждого набора атрибутов.
15.	Замерять среднее время поиска — 1000 случайных Get для каждой стратегии × каждого n × каждого атрибута.
16.	Замерять на одних и тех же данных: для каждого n — свой CSV-файл (persons_N.csv), один и тот же для всех трёх стратегий.
17.	Дополнительно замерять на отсортированных данных (data_order = "sorted"): сортировка префикса в памяти (std::sort), не часть замера. Демонстрирует деградацию BST до O(n).
18.	Выгружать результаты в CSV: strategy, attribute, data_order, n, build_ms, avg_search_us, collisions.
19.	Выгружать результаты в CSV через диалог выбора файла (QFileDialog).
20. UI: таблица сравнения (QTableView) — строки по strategy × n × data_order, столбцы — build_ms, avg_search_us.
21.	В UI: график зависимости времени от n (QChartView) — отдельные линии для каждой стратегии.
22.	IDictionary::GetCount() — число уникальных ключей.
23.	IDictionary::GetCapacity() — вместимость (число бакетов/слотов); для TreeIndex возвращает Size().
24.	IDictionary::Add(key, value) — добавить пару; при дубликате ключа — перезапись (ключ уникален).
25.	Главное окно с вкладками: «Данные», «Индекс», «Поиск», «Сравнение», «График».
26.	Вкладка «Данные»: загрузка CSV (QFileDialog), таблица Person (QTableView), счётчик записей.
27.	Вкладка «Индекс»: выбор стратегии (QComboBox), выбор атрибутов (QListWidget с чекбоксами), кнопка «Построить», прогресс-бар.
28.	Вкладка «Поиск»: поле ввода ключа, выбор атрибута, кнопка «Найти», результат в таблице.
29.	Вкладка «Сравнение»: таблица strategy × n × data_order × build_ms × avg_search_us, кнопка «Экспорт в CSV».
30. Отображение ошибок (неверный CSV, отсутствующий ключ) — через QMessageBox.




Что происходит при работе программы:
1. Данные лежат в CSV:
firstName,middleName,lastName,birthYear
Иван,Иванович,Иванов,1980
Пётр,Петрович,Петров,1975
...

2. CsvLoader читает файл -> ArraySequence<Person> data:
data.Get(0) -> Person("Иван", "Иванович", "Иванов", 1980)
data.Get(1) -> Person("Пётр", "Петрович", "Петров", 1975)
 ...
data.GetCount() -> 10000

3. PersonIndexer::Build(data, {"fullName"}) -> IDictionary<string, int>
(последовательность Person прошла через индекс):
"Иванов Иван Иванович" -> 0
"Петров Пётр Петрович" -> 1
"Сидоров Сидор Сидорович" -> 2
...