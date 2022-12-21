#include <iostream>
#include <tuple>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

template <typename Type>
Type setArgument(std::string inputValue)
{
	Type arg;  std::stringstream buffer;

	buffer << inputValue;
	buffer >> arg;

	return arg;
}

template <typename Ch, typename Tr, size_t I, typename... Args>
class TuplePrinter
{
public:
	static void print(std::basic_ostream<Ch, Tr>& os, const std::tuple<Args...>& t)
	{
		TuplePrinter<Ch, Tr, I - 1, Args...>::print(os, t);
		if (I < sizeof...(Args))
			os << " ";
		os << std::get<I>(t);
	}
};

template <typename Ch, typename Tr, typename... Args>
class TuplePrinter<Ch, Tr, 0, Args...>
{
public:
	static void print(std::basic_ostream<Ch, Tr>& os, const std::tuple<Args...>& t)
	{
		os << std::get<0>(t);
	}
};

template <typename Ch, typename Tr, typename... Args>
class TuplePrinter<Ch, Tr, -1, Args...>
{
public:
	static void print(std::basic_ostream<Ch, Tr>& os, const std::tuple<Args...>& t) {}
};

template <typename Ch, typename Tr, typename... Args>
auto operator<<(std::basic_ostream<Ch, Tr>& os, const std::tuple<Args...>& t)
{
	TuplePrinter<Ch, Tr, sizeof...(Args) - 1, Args...>::print(os, t);
}

template <typename... Types>
class CSVParser {
public:
	~CSVParser() = default;
	CSVParser(std::ifstream& file, int number_skip, char separator = ',', char screening_data = '\"'): data(file)
	{
		this->separator = separator;
		this->screening_data = screening_data;
		
		for (number_skip; number_skip>=0; number_skip--) {
			std::string curr;
			std::getline(file, curr);
		}
		this ->buffer = this->getdata();
	}
	std::tuple<Types...> getdata()
	{
		char currentSymb = 0;

		std::string					readstr;
		std::vector<std::string>	splittedstr;

		std::vector<std::string> output = getstr();

		auto f = [&output]() -> std::string
		{
			std::string s = output.back();
			output.pop_back();
			return s;
		};
		if (this->isEof)
			exit(0);
		if (output.size() != sizeof...(Types))	
			throw;
		if (isScreen == true)
			throw;

		return std::tuple<Types...>(setArgument<Types>(f())...);
	}
	class CSVInputIterator
	{
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = std::tuple<Types...>;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		CSVInputIterator(CSVParser<Types...>* _object = nullptr) : object(_object) {};
		CSVInputIterator(const CSVInputIterator&) = default;
		CSVInputIterator& operator=(const CSVInputIterator&) = default;

		friend void swap(CSVInputIterator& a, CSVInputIterator& b) { std::swap(a.object, b.object); }

		friend bool operator==(CSVInputIterator& a, CSVInputIterator& b) { return a.object == b.object; }
		friend bool operator!=(CSVInputIterator& a, CSVInputIterator& b) { return a.object != b.object; }

		value_type operator*() { return object->buffer; }
		CSVInputIterator& operator++()
		{
			try
			{
				object->buffer = object->getdata();
			}
			catch (std::runtime_error& e)
			{
				object = nullptr;
			}

			return *this;
		}

		CSVParser<Types...>* object;
	};

	CSVInputIterator begin() { return CSVInputIterator(this); }
	CSVInputIterator end() { return CSVInputIterator(); }
private:
	char separator;
	char screening_data;
	std::ifstream& data;

	bool isScreen = false;
	bool isEof = false;
	std::tuple<Types...> buffer;

	std::vector<std::string> getstr() {
		std::vector<std::string> lines;
		std::string curr;
		std::vector <std::string>	str;
		getline(data, curr);

		int end = 0;
		int start;
		while ((start = curr.find_first_not_of(separator, end)) != std::string::npos)
		{
			end = curr.find(separator, start);
			lines.push_back(curr.substr(start, end - start));
		}

		std::string value;
		for (auto x : lines) {
			value += x;
			if (x.find(screening_data) != std::string::npos)
				isScreen = !isScreen;
			if (isScreen == false)
			{
				str.push_back(value);
				value = "";
			}
			if (isScreen)
			{
				value += separator;
			}
			
		}
		if (this->data.eof()) this->isEof = true;

		return str;
	}
};

int main()
{
	std::ifstream fin("cinema_sessions.csv");
	//пишем тип то что у нас в самом файле
	CSVParser<int, int, std::string> p(fin, 3, ',', '/');

	//выводим строчки
	for (auto x : p)
	{
		std::cout << x;
		std::cout << std::endl;
	}

	return 0;
}