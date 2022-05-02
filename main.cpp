#include <fstream>
#include <iostream>
#include <cstdlib>
#include <list>
#include <cmath>

using namespace std;

struct CodeChar{

	char ch;
	float probability;
	unsigned long long code;
	int top;
};

struct CodeTable
{
	int size;
	CodeChar* codeChar;
};

struct ItemDB //структура базы данных
{
	char author[12];
	char title[32];
	char publisher[16];
	short int year;
	short int pages;
};

struct ListItem //элемент списка
{
	ItemDB* data;
	ListItem * next; //ссылка на след.элемент
};

void printHeadDataBase(ListItem* db, int offset = 0, int count = 20);


class TreeNode {//храним два ключа и три возможных потомка
	int * keys; //массив ключей
	ListItem** values; //массив значений

	int t; //указание на то , сколько текущих элементов занято
	TreeNode** C;//массив потомков

	int n;//указание на то , сколько элементов хранится
	bool leaf; //является ли даннный элемент листом, те крайний он если есть потомки

public: //создание узла
	TreeNode(int t1, bool leaf1) {
		t = t1;
		leaf = leaf1;
		keys = new int[2 * t - 1];
		values = new ListItem * [2 * t - 1];

		for (int i=0; i < 2*t-1; i++)
		{
			keys[i] = 0;
			values[i] = 0;
		}

		C = new TreeNode * [2 * t];

		for (int i = 0; i < 2 * t; i++)
		{
			C[i] = 0;
		}

		n = 0;
	}

	void insertNonFull(int k, ItemDB * value) { //вставляем узел
		int i = n - 1;

		if (leaf == true) {
			while (i >= 0 && keys[i] > k) {
				keys[i + 1] = keys[i];
				values[i + 1] = values[i];
				values[i] = 0;
				i--;
			}

			keys[i + 1] = k;

			ListItem* tmp = new ListItem;
			tmp->data = value;
			tmp->next = values[i + 1];
			values[i + 1] = tmp;

			n = n + 1;
		}
		else {
			while (i >= 0 && keys[i] > k)
				i--;

			if (C[i + 1]->n == 2 * t - 1) {
				splitChild(i + 1, C[i + 1]);

				if (keys[i + 1] < k)
					i++;
			}
			C[i + 1]->insertNonFull(k, value);
		}
	}

	void splitChild(int i, TreeNode* y) //разделяем дите
	{
		TreeNode* z = new TreeNode(y->t, y->leaf);
		z->n = t - 1;

		for (int j = 0; j < t - 1; j++)
		{
			z->keys[j] = y->keys[j + t];
			z->values[j] = y->values[j + t];
		}


		if (y->leaf == false) {
			for (int j = 0; j < t; j++)
				z->C[j] = y->C[j + t];
		}

		y->n = t - 1;
		for (int j = n; j >= i + 1; j--)
			C[j + 1] = C[j];

		C[i + 1] = z;

		for (int j = n - 1; j >= i; j--)
		{
			keys[j + 1] = keys[j];
			values[j + 1] = values[j];
		}


		keys[i] = y->keys[t - 1];
		values[i] = y->values[t - 1];
		n = n + 1;
	}

	void traverse() //выполняет обход дерева вводя его элементы на экран, идет от крайнего левого потомка к корню и  крайнему правому потомку
	{
		int i;
		for (i = 0; i < n; i++) {
			if (leaf == false)
				C[i]->traverse();

			if (C[i] == 0)
				return;

			for (int k=0; k<C[i]->n; k++)
			{
				printHeadDataBase(C[i]->values[k], 0, -1);
			}
		}

		if (leaf == false)
			C[i]->traverse();
	}


	TreeNode* search(int k) //поиск между ключами элемент находим, либо левее, либо правее, если совпадение с ключом оттуда и берем
	{ //и это несколько элементов
		int i = 0;
		while (i < n && k > keys[i])
			i++;

		if (keys[i] == k)
			return this;

		if (leaf == true)
			return NULL;

		return C[i]->search(k);
	}

	friend class BTree;
};

class BTree { //это всего лишь вспомогательный класс, который находится около основного класса, это TreeNode
	TreeNode* root;
	int t;

	bool insertExistingList(int key, ItemDB* item)
	{
		TreeNode* node = root->search(key);

		if (node == 0)
			return false;
		
		ListItem* tmp = new ListItem;
		tmp->data = item;
		tmp->next = 0;

		for(int i=0; i < node->n; i++)
		{
			if (node->keys[i] == key)
			{
				tmp->next = node->values[i];
				node->values[i] = tmp;
				break;
			}
		}

		return true;
	}

public:
	BTree(int temp) {
		root = NULL;
		t = temp;
	}

	void traverse() {
		if (root != NULL)
			root->traverse();
	}

	ListItem * search(int k) {
		if (root == 0)
			return 0;

		TreeNode * node = root->search(k);

		if (node == 0)
			return 0;

		for (int i=0; i < node->n; i ++)
		{
			if (node->keys[i] == k)
				return node->values[i];
		}
		return 0;
	}

	void insert(int k, ItemDB * item) //все элементы вставляем сюда, ключ это год, вставляем целый элемент из бд
	{
		if (root == NULL) {
			ListItem* tmp = new ListItem;
			tmp->data = item;
			tmp->next = 0;

			root = new TreeNode(t, true); //изначально вставляем всё в root, после того, как root заполнен 
			root->keys[0] = k; //мы пытаемся по принципу обычного дерева поиска, либо влево, либо между двумя ключами вставлять элемент либо вправо
			root->values[0] = tmp; //опять разбиваем на потомков
			root->n = 1;
		}
		else {
			if (insertExistingList(k, item))
			{
				return;
			}

			if (root->n == 2 * t - 1) {
				TreeNode* s = new TreeNode(t, false);

				s->C[0] = root;

				s->splitChild(0, root);

				int i = 0;
				if (s->keys[0] < k)
					i++;
				s->C[i]->insertNonFull(k, item);

				root = s;
			}
			else
				root->insertNonFull(k, item);
		}
	}
};



ItemDB** toArray(ListItem* h, int size) //преобразование в динамический массив, взяли все элементы из списка и все элементы перенесли в массив
{
	ItemDB** arr = new ItemDB * [size]; //каждый элемент списка это указатель на структуру
	int pos = 0;
	for (ListItem * cur = h; cur != 0; cur = cur->next) //каждый элемент заносим в cur и из cur указываем ссылку на саму бд
	{
		arr[pos] = cur->data; //запихиваем в массив
		pos++;
	}
	return arr;
}


void deleteArray(ItemDB** arr) //удаление самого массива, без удаления ложных элементов
{
	delete[] arr;
}

int getLenStr(char * str) //функция измеряющая длину строки
{
	int i = 0;
	while(str[i]!=0) //код=0 значит конец строки
	{
		i++;
	}
	return i;
}

int cmpStr(char * str1, char * str2, int n=-1) //функция для сравнения строк
{
	int len1 = getLenStr(str1);
	int len2 = getLenStr(str2);
	//проходимся по обоим строкам одновременно, сравниваем каждый символ друг с другом
	int pos = 0;

	while (pos < len1 && pos < len2)//если код символа меньше , чем код символа во второй строке, то строка номер один будет меньше
	{//если все сиволы совпали это не значит, что они равны, у них могут отличаться длины строк
		if (str1[pos] < str2[pos])
		{
			return -1; //значит меньше первая строка
		}
		else if(str1[pos] > str2[pos])
		{
			return 1; //значит больше первая строка
		}
		pos++;
		n--;
		if (n == 0) //равны друг другу
			return 0;
	}

	if (len1 < len2) //длина строк
	{
		return -1; //идет раньше
	}
	else if (len1 > len2)
	{
		return 1; //идет позже строка
	}

	return 0; //если длины равны то возвращаем ноль, значит они равны
}

int cmpItemDB(ItemDB * left, ItemDB * right) //функция для сравнения двух элементов
{
	int res = cmpStr(left->publisher, right->publisher); //сравниваем издательство, если они равны, то сравниваем авторов, если строка слева меньше или больше чем справа, то все ок
	if (res != 0) //если не одинаковы издательства, то возвращаем, что нам вернула функция сравнения
	{
		return res;
	}
	res = cmpStr(left->author, right->author);
	return res;
}

ListItem * merge(ListItem * h1, ListItem * h2) //берем первый элемент списка h1 и первый элемент списка h2
{
	ListItem * t1 = 0;
	ListItem * t2 = 0;
	ListItem * temp = 0;

	// Возврат если первый список пуст
	if (h1 == NULL)
		return h2;

	// Возврат если второй список пуст
	if (h2 == NULL)
		return h1;

	t1 = h1;

	// Цикл для обхода второго списка, чтобы объединить узлы в h1 в отсортированном виде.
	while (h2 != NULL)
	{
		// Принимая головной узел второго списка как t2.
		t2 = h2;

		// Смещение второй главы списка к следующей.
		h2 = h2->next;
		t2->next = NULL;

		// Если значение данных меньше, чем заголовок первого списка, добавьте этот узел в начало.
		if (cmpItemDB(h1->data, t2->data) == 1)
		{
			t2->next = h1;
			h1 = t2;
			t1 = h1;
			continue; //внутрь существующего списка вставляем, не создавая нового
		}

		// Пройдитесь по первому списку.
	flag:
		if (t1->next == NULL)
		{
			t1->next = t2;
			t1 = t1->next;
		}
		// Перемещайтесь по первому списку до тех пор, пока t2->data больше, чем данные узла.
		else if (cmpItemDB((t1->next)->data, t2->data) <=0 )
		{
			t1 = t1->next;
			goto flag;
		}
		else
		{
			// Вставьте узел, так как t2->data меньше, чем следующий узел.
			temp = t1->next;
			t1->next = t2;
			t2->next = temp;
		}
	}

	// Возвращает начало нового отсортированного списка.
	return h1;
}

int findPosition(ItemDB** data, int size, char* key) //бинарный поиск
{
	int left = 0;
	int right = size-1;

	while (left <= right)
	{
		int mid = (left + right) / 2;
		int res = cmpStr(data[mid]->publisher, key, 3);

		if (res == 1)
		{
			right = mid - 1;
		}
		else if(res == -1)
		{
			left = mid + 1;
		}
		else
		{
			return mid;
		}
	}

	return -1;
}


ListItem* filter(ItemDB ** data, int size, char * key) //принимает отсортированные данные, размер данных и сам ключ
{
	ListItem * head=0;
	ListItem * tail=0;

	int pos = findPosition(data, size, key); //ищем все элементы с нужным ключом

	if (pos == -1)
		return 0;

	int s = pos - 1; //выясняем границы элементов которые нам нужны и запихиваем в список
	int e = pos + 1;
	
	head = tail = new ListItem; //всё что слева в голову, а справа в конец
	head->data = data[pos];
	head->next = 0;

	while (s>=0 && cmpStr(data[s]->publisher, key, 3) == 0)
	{
		ListItem * temp = new ListItem;
		temp->data = data[s];
		temp->next = head;
		head = temp;
		s--;
	}

	while (e < size && cmpStr(data[e]->publisher, key, 3) == 0)
	{
		ListItem* temp = new ListItem;
		temp->data = data[e];
		temp->next = 0;
		tail->next = temp;
		tail = temp;
		e++;
	}

	return head; // нашли все вернули  голову полученного списка
}


// Функция, реализующая сортировку слиянием в связанном списке с использованием ссылки.
void mergeSort(ListItem ** head)
{
	ListItem* first = 0;
	ListItem * second = 0;
	ListItem * temp = 0;
	first = *head;
	temp = *head;

	// Возврат, если в списке меньше двух узлов.
	if (first == NULL || first->next == NULL)
	{
		return;
	}
	else
	{
		// Разбейте список на две половины в качестве первого и второго в качестве заголовка списка.
		while (first->next != NULL)
		{
			first = first->next;
			if (first->next != NULL)
			{
				temp = temp->next;
				first = first->next;
			}
		}
		second = temp->next;
		temp->next = NULL;
		first = *head;
	}

	// Реализация подхода «разделяй и властвуй».
	mergeSort(&first);
	mergeSort(&second);

	// Объедините две части списка в отсортированный.    
	*head = merge(first, second);
}

ListItem * readDataBase(const char const* dbName, int countItemDB) //чтение файла
{
	

	ifstream in(dbName, ifstream::binary); //открываем файл на чтение в бинарном виде
	ListItem* head , * tail;
	head = tail = new ListItem; //создаем фейковую голову
	head->next = 0;

	for(int i=0; i < countItemDB; i++) //читаем данные из файла
	{
		ItemDB* item = new ItemDB; //создаем объект соответствующий нашей структуре, выделяем память и место под нее
		in.read((char*)item, sizeof(ItemDB)); //читаем данные и запихиваем в item 
		tail->next = new ListItem; //заполняем элементы за хвостом
		tail->next->data = item;
		tail->next->next = 0;
		tail = tail->next;
	}

	in.close();
	ListItem* temp = head; //забываем об этой фейк голове и удаляем ее и возвращаем реальную голову
	head = head->next;
	delete temp;
	return head;
}

void clearMemoryDataBase(ListItem * db)
{
	ListItem* cur = db;

	while (cur!=0)
	{
		ListItem* next = cur->next;
		memset(cur->data, 0, sizeof(ItemDB));
		delete cur->data;
		delete cur;
		cur = next;
	}
}

void printItem(ItemDB* item, int num)
{
	cout << num << " ";
	cout << item->author << " ";
	cout << item->title << " ";
	cout << item->publisher << " ";
	cout << item->year << " ";
	cout << item->pages << " ";
	cout << endl;
}

void printHeadDataBase(ListItem * db, int offset, int count ) //возможность пропустить элементы offset, count-вывести сколько необходимо
{
	int ind = 0; //на какой позиции,чтобы соответствовать offset
	ListItem* cur = db;
	for (;cur != 0 && ind < offset; cur = cur->next)//пока индекс меньше офсета, переходи к след.элементу нашего списка
	{
		ind++;
	}

	for (;cur!=0 && count--; cur=cur->next)//вывели ли все нужные элементы на экран и смотрим до конца
	{
		ItemDB * item = cur->data;
		printItem(item, ind + 1);//вывод на экран
		ind++;
	}
}

int countDoubleChar(ListItem* dataBase, CodeChar * arrCounter)
{
	int count = 0;

	for (ListItem * cur = dataBase; cur!=0; cur = cur -> next)
	{
		char * arrField[] = { cur->data->author, cur->data->publisher, cur->data->title};
		int lenArrField = sizeof(arrField) / sizeof(char*);
		for (int i=0; i < lenArrField; i++)
		{
			int lenField = getLenStr(arrField[i]);
			for (int k=0; k < lenField;  k++)
			{
				count++;
				unsigned char code = (unsigned char) arrField[i][k];
				arrCounter[code].ch = arrField[i][k];
				arrCounter[code].probability += 1;
			}
		}
	}
	return count;
}

CodeTable getTableForCharFieldWithOutCode(ListItem* dataBase) //Создайте список вероятностей или подсчетов частоты для данного набора символов, чтобы была известна относительная частота появления каждого символа.
{
	const int size = 256;
	CodeChar arrCounter[size];
	CodeTable table;
	table.size = 0;

	for (int i=0; i < size; i ++)
	{
		arrCounter[i].ch = 0;
		arrCounter[i].code = 0;
		arrCounter[i].probability = 0;
		arrCounter[i].top = 0;
	}

	int count = countDoubleChar(dataBase, arrCounter);

	for (int i=0; i < size; i++)
	{
		if (arrCounter[i].probability>0)
		{
			table.size++;
			arrCounter[i].probability /= count;
		}
	}

	table.codeChar = new CodeChar[table.size];

	for (int i=0, pos =0; i < size; i ++)
	{
		if (arrCounter[i].probability > 0)
		{
			table.codeChar[pos].ch = arrCounter[i].ch;
			table.codeChar[pos].probability = arrCounter[i].probability;
			table.codeChar[pos].code = 0;
			table.codeChar[pos].top = 0;
			pos++;
		}
	}

	return table;
}

void sortByProbability(CodeTable & table) //Отсортируйте список символов в порядке убывания вероятности, наиболее вероятные слева и наименее вероятные справа.
{ //Функция для сортировки символов по их вероятности или частоте
	for (int j = 1; j <= table.size - 1; j++) 
	{
		for (int i = 0; i < table.size - 1; i++) 
		{
			if ((table.codeChar[i].probability) > (table.codeChar[i + 1].probability)) {
				CodeChar temp;
				temp.probability = table.codeChar[i].probability;
				temp.ch = table.codeChar[i].ch;

				table.codeChar[i].probability = table.codeChar[i + 1].probability;
				table.codeChar[i].ch = table.codeChar[i + 1].ch;

				table.codeChar[i + 1].probability = temp.probability;
				table.codeChar[i + 1].ch = temp.ch;
			}
		}
	}
}

void shannon(int l, int h, CodeTable & table)//Разделите список на две части так, чтобы суммарная вероятность того, что обе части были максимально близки друг к другу.
//Присвойте значение 0 левой части и 1 правой части.
{
	if (l >= h)
		return;

	if ((l + 1) == h) // 10(10) = 1010(2)  1010 << 1 -> 10100
	{
		table.codeChar[h].top+=1;
		table.codeChar[h].code = (table.codeChar[h].code << 1) | 0; //присваиваем 0 для левой части
		table.codeChar[l].top += 1;
		table.codeChar[l].code = (table.codeChar[l].code << 1) | 1; //присваиваем 1 для правой части
	}
	else 
	{
		float pack1 = 0, pack2 = 0, diff1 = 0, diff2 = 0;
		int i=0, d=0, k=0, j=0;

		for (i = l; i <= h - 1; i++)
			pack1 = pack1 + table.codeChar[i].probability;

		pack2 = pack2 + table.codeChar[h].probability;

		diff1 = pack1 - pack2;
		if (diff1 < 0)
			diff1 = diff1 * -1;
		j = 2;
		while (j != h - l + 1) {
			k = h - j;
			pack1 = pack2 = 0;
			for (i = l; i <= k; i++)
				pack1 = pack1 + table.codeChar[i].probability;
			for (i = h; i > k; i--)
				pack2 = pack2 + table.codeChar[i].probability;
			diff2 = pack1 - pack2;
			if (diff2 < 0)
				diff2 = diff2 * -1;
			if (diff2 >= diff1)
				break;
			diff1 = diff2;
			j++;
		}
		k++;

		for (i = l; i <= k; i++)
		{
			table.codeChar[i].top += 1;
			table.codeChar[i].code = (table.codeChar[i].code << 1) | 1;
		}

		for (i = k + 1; i <= h; i++)
		{
			table.codeChar[i].top += 1;
			table.codeChar[i].code = (table.codeChar[i].code << 1) | 0;
		}
		//вызываем функцию
		shannon(l, k, table);
		shannon(k + 1, h, table);
	}
}

void calcCodeShannonFano(CodeTable & table)
{
	sortByProbability(table);
	shannon(0, table.size-1, table);
}

ListItem* encodeDataBase(CodeTable & table, ListItem* dataBase)
{
	return 0;
}

list<int> toBitArray(CodeChar code)
{
	list<int> res;
	unsigned long long v = code.code;
	for (int i = 0; i< code.top ; i++)
	{
		res.push_front(v % 2);
		v = v / 2;
	}
	return res;
}

void showCodeTableProbabilityCharacter(CodeTable & table)
{
	for (int i=0; i < table.size; i++)
	{
		cout << table.codeChar[i].ch << ": "
			<< table.codeChar[i].probability
			<< " code: ";
		for (int k: toBitArray(table.codeChar[i]))
		{
			cout << k;
		}
		cout << endl;
	}
}

void showCodeTableProbabilityByte(CodeTable& table)
{
	for (int i = 0; i < table.size; i++)
	{
		cout << (int)(unsigned char)table.codeChar[i].ch << ": " << table.codeChar[i].probability<< endl;
	}
}

CodeTable getTableForDB(const char * dbName) //сами символы еще не считает, считает вероятности
{
	const int size = 256;
	CodeChar arrCounter[size];
	CodeTable table;
	table.size = 0;
	ifstream in(dbName, ifstream::binary);

	in.seekg(0, in.end);
	int sizeFile = in.tellg();
	in.seekg(0, in.beg);
	unsigned char* data = new unsigned char[sizeFile];
	in.read((char*)data, sizeFile);
	in.close();

	for (int i = 0; i < size; i++)
	{
		arrCounter[i].ch = 0;
		arrCounter[i].code = 0;
		arrCounter[i].probability = 0;
		arrCounter[i].top = 0;
	}

	for (int i = 0; i < sizeFile; i++)
	{
		int pos = (unsigned char)data[i];
		arrCounter[pos].ch = (char)data[i];
		arrCounter[pos].probability += 1;
	}

	for (int i = 0; i < size; i++)
	{
		if (arrCounter[i].probability > 0)
		{
			table.size++;
			arrCounter[i].probability /= sizeFile;
		}
	}

	table.codeChar = new CodeChar[table.size];

	for (int i = 0, pos = 0; i < size; i++)
	{
		if (arrCounter[i].probability > 0)
		{
			table.codeChar[pos].ch = arrCounter[i].ch;
			table.codeChar[pos].probability = arrCounter[i].probability;
			table.codeChar[pos].code = 0;
			table.codeChar[pos].top = 0;
			pos++;
		}
	}
	delete[] data;
	return table;
}

double calcEntropy(CodeTable& table)
{
	double result = 0;
	for (int i=0; i<table.size; i++)
	{
		result -= table.codeChar[i].probability * log2(table.codeChar[i].probability);
	}
	return result;
}

double meanLenCode(CodeTable& table)
{
	double result = 0;
	for (int i = 0; i < table.size; i++)
	{
		result += table.codeChar[i].top;
	}
	result /= table.size;
	return result;
}

CodeChar getCodeFromTable(CodeTable& table, char ch)
{
	for (int i = 0; i < table.size; i++)
	{
		if (table.codeChar[i].ch == ch)
			return table.codeChar[i];
	}
	throw exception("Tabel has not character");
}

void encodeFile(const char * dbName, const char * encodedFile, CodeTable & table)
{//мы подразумеваем, что наш код эффективен и мб съест бд, тк каке=ие-то символы в русском языке встрчеаются чаще, а что-то нет и поэтому должно быть эффективно сжато все
	typedef unsigned int uint;

	ifstream in(dbName, ifstream::binary);

	in.seekg(0, in.end);
	int sizeFile = in.tellg();
	in.seekg(0, in.beg);
	unsigned char* data = new unsigned char[sizeFile];
	in.read((char*)data, sizeFile);
	in.close();

	uint *  encodedData = new uint[sizeFile]; //выделяем буффер размером не меньше, чем исходный файл, 32 бита, закидываем биты наших кодовых последовательностей
	uint encodedBit = 0;
	for (int i=0; i < sizeFile; i++)
	{
		encodedData[i] = 0;
	}

	int cellSizeBit = sizeof(uint)*8; //заполняем по максимуму буфер, производим перенос в основной буфер и так далее

	for (int i = 0; i < sizeFile; i++)
	{
		CodeChar code = getCodeFromTable(table, data[i]);

		int position = encodedBit / cellSizeBit;
		int countBusyBit = encodedBit % cellSizeBit;
		int countFreeBit = cellSizeBit - countBusyBit;

		if (countFreeBit >= code.top) //если остались свободные биты мы их закидываем заново,
		{
			int countZero = countFreeBit - code.top;
			uint mask = code.code;
			mask = mask << countZero;
			encodedData[position] |= mask;
		}
		else
		{
			int rightShift = code.top - countFreeBit;
			uint mask = code.code;
			mask = mask >> rightShift;
			uint mask2 = code.code;
			int letfShift = cellSizeBit - rightShift;
			mask2 = mask2 << letfShift;

			encodedData[position] |= mask;
			encodedData[position+1] |= mask2;
		}
		encodedBit += code.top;
	}



	int encodeByte = ((encodedBit - 1) / 8) + 1; //производим запись данных, по количеству бит в большую сторону 

	ofstream fout(encodedFile, ifstream::binary);
	fout.write((char*)encodedData, encodeByte);
	fout.close();

	delete[] data;
	delete[] encodedData;
}

void menu()
{
		cout << "select action:" << endl;
		cout << "1. show top 20" << endl;
		cout << "2. sorted data" << endl;
		cout << "3. find by key" << endl;
		cout << "4. build tree" << endl;
		cout << "5. find into tree" << endl;
		cout << "6. show code table" << endl;
		cout << "7. show avg len code word" << endl;
		cout << "8. save compress file" << endl;
		cout << "0. exit" << endl;
}


int main()
{
	
	const int countItemDB = 4000;
	const char* dbName = "testBase1.dat";
	const char* encodeDbName = "testBase1_encode.dat";
	ListItem* dataBase = readDataBase(dbName, countItemDB);
	ItemDB** dataArray=0;
	ListItem* filtredData=0;
	BTree tree(2);
	CodeTable table = getTableForDB(dbName);
	calcCodeShannonFano(table); 

	int action = 0;

	while (1)
	{
		menu();
		cin >> action;

		if (action == 1)
		{
			printHeadDataBase(dataBase, 0, 20);
		}
		else if (action == 2)
		{
			mergeSort(&dataBase);
		}
		else if (action == 3)
		{
			if (filtredData != 0)
			{
				clearMemoryDataBase(filtredData);
			}
			dataArray = toArray(dataBase, countItemDB);
			char key[] = "ЂаеЁЇ®ў Ltd  ";
			filtredData = filter(dataArray, countItemDB, key);
			printHeadDataBase(filtredData, 0, countItemDB);
		}
		else if (action == 4)
		{
			if (filtredData != 0)
			{
				for (ListItem * tmp = filtredData; tmp!=0; tmp = tmp->next)
				{
					tree.insert(tmp->data->year, tmp->data);
				}
				tree.traverse();
			}
			else
			{
				cout << "to do 3 " << endl;
			}
		}
		else if (action == 5)
		{
			printHeadDataBase(tree.search(1990), 0, -1); //смотрим где находится значение, значение хранимое по ключу-это несколько элементов
		}
		else if (action == 6)
		{

			showCodeTableProbabilityCharacter(table);
		}
		else if (action == 7)
		{
			cout << "entropy file: " << calcEntropy(table) << endl;
			cout << "mean len code: " << meanLenCode(table) << endl;
		}
		else if (action == 8)
		{
			encodeFile(dbName, encodeDbName, table);
		}
		else if (action == 0)
		{
			break;
		}
	}
	
	//ListItem * dataBase = readDataBase(dbName, countItemDB);
	////CodeTable table = getTableForCharFieldWithOutCode(dataBase);
	//CodeTable table = getTableForDB(dbName);
	//calcCodeShannonFano(table);
	//showCodeTableProbabilityCharacter(table);
	//cout << "entropy file: " << calcEntropy(table) << endl;
	//cout << "mean len code: " << meanLenCode(table) << endl;
	//encodeFile(dbName, encodeDbName, table);

	////mergeSort(&dataBase);

	////printHeadDataBase(dataBase, 0, 100);

	////ItemDB ** dataArray = toArray(dataBase, countItemDB);

	////printItem(dataArray[0], 1);
	////printItem(dataArray[1], 2);
	////printItem(dataArray[2], 3);
	////printItem(dataArray[3], 4);

	//char key[] = "ЂаеЁЇ®ў Ltd  ";

	//ListItem * filtredData = filter(dataArray, countItemDB, key);

	//printHeadDataBase(filtredData, 0, countItemDB);

	/*BTree tree(2);
	for (ListItem * tmp = filtredData; tmp!=0; tmp = tmp->next)
	{
		tree.insert(tmp->data->year, tmp->data);
	}*/

	//tree.traverse();

	//printHeadDataBase(tree.search(1990), 0, -1);


	//clearMemoryDataBase(dataBase);
	//deleteArray(dataArray);

	return 0;
}