#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "json.h"

namespace json {

	class DictKeyPart;
	class DictValuePart;
	class ArrayPart;

	// Конструктор JSON и методы работы с ним
	class Builder {
	public:
		Builder() = default;
		Builder(Builder&& other);
		Builder& operator=(Builder&&);

		Builder(const Builder&) = delete;
		Builder& operator=(const Builder&) = delete;

		Builder& Key(std::string);
		Builder& Value(Node::Value);
		DictKeyPart StartDict();
		Builder& EndDict();
		ArrayPart StartArray();
		Builder& EndArray();
		Node Build();

	private:
		// Верхняя нода в стеке - текущий собираемый массив или словарь.
		std::vector<Node> stack_;

		// `true`, если значения идут в словарь 
		bool expect_key_ = false;

		// `true` после сборки ноды верхнего уровня
		bool finished_ = false;

		// Стек названий ключей во вложенных собираемых словарях
		std::vector<std::string> key_stack_;

		// `true`, если значение данного конструктора было перемещено в другую переменную
		bool moved_out_of_ = false;

		void Swap(Builder& other);
	};

	// Общий класс для вспомогательных классов - конструкторов частей JSON нод
	class PartBuilder {
	protected:
		PartBuilder(Builder&& builder) 
			: builder_(std::move(builder)) {
		}
		Builder builder_;

		Builder& Key(std::string);
		Builder& Value(Node::Value);
		DictKeyPart StartDict();
		Builder& EndDict();
		ArrayPart StartArray();
		Builder& EndArray();
		Node Build();

	};

	// Часть конструктора JSON ( `Key()` - `Value()` / окончание словаря )
	class DictKeyPart : private PartBuilder {
		using PartBuilder::PartBuilder;

	public:
		DictKeyPart(Builder&& builder)
			: PartBuilder(std::move(builder)) {
		}

		DictKeyPart(const DictKeyPart&) = delete;
		DictKeyPart& operator=(const DictKeyPart&) = delete;

		DictValuePart Key(std::string);
		Builder EndDict();
	};

	// Часть конструктора JSON ( `Value()` / начало вложенного словаря или массива )
	class DictValuePart : private PartBuilder {
		using PartBuilder::PartBuilder;

	public:
		DictValuePart(Builder&& builder)
			: PartBuilder(std::move(builder)) {
		}

		DictValuePart(const DictValuePart&) = delete;
		DictValuePart& operator=(const DictValuePart&) = delete;
		DictValuePart& operator=(DictValuePart&&) = delete;

		DictKeyPart Value(Node::Value);
		DictKeyPart StartDict();
		ArrayPart StartArray();
	};

	// Часть конструктора JSON ( элемент массива / окончание массива )
	class ArrayPart : private PartBuilder {
		using PartBuilder::PartBuilder;

	public:
		ArrayPart(Builder&& builder)
			: PartBuilder(std::move(builder)) {
		}

		ArrayPart(const ArrayPart&) = delete;
		ArrayPart& operator=(const ArrayPart&) = delete;

		ArrayPart& Value(Node::Value);
		DictKeyPart StartDict();
		ArrayPart StartArray();
		Builder EndArray();
	};

}  // namespace json
