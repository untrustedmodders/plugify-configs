#pragma once
#include <plugify-configs/methods.hpp>
#include <plugify-configs/plugify-configs.hpp>
#include <plugify/vector.hpp>
#include <plugify/string.hpp>
#include <map>
#include <deque>
#include <variant>
#include <memory>

namespace pcf
{
	class Config::Detail
	{
	private:
		class Node
		{
		public:
			using Ptr = std::unique_ptr<Node>;
			using NullType = std::nullptr_t;
			using BoolType = bool;
			using NumberType = int64_t;
			using FloatType = double;
			using StringType = plg::string;
			using ObjectType = std::multimap<plg::string, Ptr>;
			using ArrayType = plg::vector<Ptr>;
			using StorageType = std::variant<NullType, BoolType, NumberType, FloatType, StringType, ObjectType, ArrayType>;
			template<class T>
			static constexpr bool is_storable_v = std::is_same_v<T, NullType> || std::is_same_v<T, BoolType> || std::is_same_v<T, NumberType> 
				|| std::is_same_v<T, FloatType> || std::is_same_v<T, StringType> || std::is_same_v<T, ObjectType> || std::is_same_v<T, ArrayType>;

		public:
			static Node::Ptr NewNode();
			static Node::Ptr NewNullNode();

		public:
			template<class SoruceType>
				requires std::is_same_v<Config::Detail::Node::Ptr, std::remove_cvref_t<SoruceType>>
			static void MergeNodes(Node::Ptr& destanation, SoruceType&& source);
			static void MergeObject(Node::Ptr& destanation, const Node::Ptr& source, const ObjectType& objectSource);
			static void MergeObject(Node::Ptr& destanation, Node::Ptr&& source, ObjectType& objectSource);
			static void MergeArray(Node::Ptr& destanation, const Node::Ptr& source, const ArrayType& arrSource);
			static void MergeArray(Node::Ptr& destanation, Node::Ptr&& source, ArrayType& arrSource);

			NodeType GetType() const;

			template<class T>
			void Set(T value);
			void SetObject();
			void SetArray();

			template<class T>
			bool TrySetFrom(T value);

			Node* PushBack();
			Node* PushBackNull();

			bool Get(bool defaultValue) const;
			int64_t Get(int64_t defaultValue) const;
			double Get(double defaultValue) const;
			plg::string Get(std::string_view defaultValue) const;

			bool Has(std::string_view key) const;
			bool Empty() const;
			int64_t GetSize() const;
			plg::string GetName(Node* node) const;

			Node* At(std::string_view key);
			Node* At(int n);
			Node* First();
			Node* Last();
			Node* Next(Node* node);
			Node* Prev(Node* node);

			Node* Create(std::string_view key);
			std::pair<Node::Ptr, Node*> Extract(Node* node);

			plg::string ToJsonString() const;

		private:
			Node();
			Node(nullptr_t);

		private:
			StorageType _storage = ObjectType{};
			ObjectType::iterator _objCache = std::get<ObjectType>(_storage).end();
			ArrayType::iterator _arrCache = {};
		};

	public:
		static std::unique_ptr<Config> MakeConfig();
		void Merge(const Detail& other);
		void MergeMove(Detail&& other);

		NodeType GetType() const;
		bool IsNull() const;
		bool IsBool() const;
		bool IsInt() const;
		bool IsFloat() const;
		bool IsString() const;
		bool IsObject() const;
		bool IsArray() const;

		template<typename T> requires Config::Detail::Node::is_storable_v<T>
		void Set(T value);
		void SetObject();
		void SetArray();

		template<class T> requires Config::Detail::Node::is_storable_v<T> || std::is_same_v<T, std::string_view>
		bool TrySetFrom(T value);

		void PushNull();
		void PushBool(bool value);
		void PushInt(int64_t value);
		void PushFloat(double value);
		void PushString(plg::string value);
		void PushObject();
		void PushArray();

		template<typename T>
		T Get(T defaultValue) const;
		plg::string Get(std::string_view defaultValue) const;

		bool HasKey(std::string_view key) const;
		bool Empty() const;
		int64_t GetSize() const;
		plg::string GetName() const;

		bool JumpFirst();
		bool JumpLast();
		bool JumpNext();
		bool JumpPrev();
		bool JumpKey(std::string_view key, bool create);
		bool JumpN(int n);
		bool JumpBack();
		void JumpRoot();

		int Remove();
		bool RemoveKey(std::string_view key);
		void Clear();

		plg::string NodeToJsonString() const;
		plg::string RootToJsonString() const;

	private:
		Detail();

		Node& GetCurrent() const;
		Node* GetCurrentParent() const;

	private:
		Node::Ptr _root = Node::NewNode();
		std::deque<Node*> _track;
	};

	template<class T>
	void Config::Detail::Node::Set(T value)
	{
		static_assert(is_storable_v<T>);
		_storage = std::move(value);
	}

	template <typename T> requires Config::Detail::Node::is_storable_v<T>
	void Config::Detail::Set(T value)
	{
		GetCurrent().Set(std::move(value));
	}
}
