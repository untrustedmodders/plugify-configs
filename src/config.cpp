#include "config.hpp"
#include <string>

namespace pcf
{
	template<typename VariantType, typename T, std::size_t index = 0>
	constexpr std::size_t variant_index() {
		static_assert(std::variant_size_v<VariantType> > index, "Type not found in variant");
		if constexpr (index == std::variant_size_v<VariantType>) {
			return index;
		}
		else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) {
			return index;
		}
		else {
			return variant_index<VariantType, T, index + 1>();
		}
	}

	Config::Detail::Node::Node() = default;

	Config::NodeType Config::Detail::Node::GetType() const
	{
		static_assert(static_cast<size_t>(NodeType::Null) == 0 && variant_index<StorageType, NullType>() == 0);
		static_assert(static_cast<size_t>(NodeType::Bool) == 1 && variant_index<StorageType, BoolType>() == 1);
		static_assert(static_cast<size_t>(NodeType::Int) == 2 && variant_index<StorageType, NumberType>() == 2);
		static_assert(static_cast<size_t>(NodeType::Float) == 3 && variant_index<StorageType, FloatType>() == 3);
		static_assert(static_cast<size_t>(NodeType::String) == 4 && variant_index<StorageType, StringType>() == 4);
		static_assert(static_cast<size_t>(NodeType::Object) == 5 && variant_index<StorageType, ObjectType>() == 5);
		static_assert(static_cast<size_t>(NodeType::Array) == 6 && variant_index<StorageType, ArrayType>() == 6);

		if (_storage.index() == std::variant_npos) {
			return NodeType::Null;
		}
		return static_cast<NodeType>(_storage.index());
	}

	void Config::Detail::Node::SetObject()
	{
		_storage = ObjectType{};
		_objCache = std::get<ObjectType>(_storage).end();
	}

	void Config::Detail::Node::SetArray()
	{
		_storage = ArrayType{};
		_arrCache = std::get<ArrayType>(_storage).end();
	}

	Config::Detail::Node* Config::Detail::Node::At(std::string_view key)
	{
		if (auto* const object = std::get_if<ObjectType>(&_storage)) {
			if (const auto it = object->find(key); it != object->end()) {
				return it->second.get();
			}
		}

		return nullptr;
	}

	Config::Detail::Node* Config::Detail::Node::First()
	{
		if (auto* const object = std::get_if<ObjectType>(&_storage)) {
			if (const auto it = object->begin(); it != object->end()) {
				_objCache = it;
				return it->second.get();
			}
		}
		else if (auto* const arr = std::get_if<ArrayType>(&_storage)) {
			if (const auto it = arr->begin(); it != arr->end()) {
				_arrCache = it;
				return it->get();
			}
		}

		return nullptr;
	}

	Config::Detail::Node* Config::Detail::Node::Last()
	{
		if (auto* const object = std::get_if<ObjectType>(&_storage)) {
			if (const auto it = object->rbegin(); it != object->rend()) {
				_objCache = it.base();
				return it->second.get();
			}
		}
		else if (auto* const arr = std::get_if<ArrayType>(&_storage)) {
			if (const auto it = arr->rbegin(); it != arr->rend()) {
				_arrCache = it.base();
				return it->get();
			}
		}

		return nullptr;
	}

	Config::Detail::Node* Config::Detail::Node::Next(Node* node)
	{
		if (auto* const object = std::get_if<ObjectType>(&_storage)) {
			if (_objCache != object->end() && _objCache->second.get() == node) {
				ObjectType::iterator nextit = _objCache;
				if (++nextit == object->end()) {
					return nullptr;
				}
				_objCache = nextit;
				return nextit->second.get();
			}

			for (auto it = object->begin(); it != object->end(); ++it) {
				if (it->second.get() == node) {
					_objCache = it;
					ObjectType::iterator nextit = it;
					if (++nextit == object->end()) {
						return nullptr;
					}
					_objCache = nextit;
					return nextit->second.get();
				}
			}

			return nullptr;
		}

		if (auto* const arr = std::get_if<ArrayType>(&_storage)) {
			if (_arrCache != arr->end() && _arrCache->get() == node) {
				ArrayType::iterator nextit = _arrCache;
				if (++nextit == arr->end()) {
					return nullptr;
				}
				_arrCache = nextit;
				return nextit->get();
			}

			for (auto it = arr->begin(); it != arr->end(); ++it) {
				if (it->get() == node) {
					_arrCache = it;
					ArrayType::iterator nextit = it;
					if (++nextit == arr->end()) {
						return nullptr;
					}
					_arrCache = nextit;
					return nextit->get();
				}
			}

			return nullptr;
		}

		return nullptr;
	}
	
	Config::Detail::Node* Config::Detail::Node::Prev(Node* node)
	{
		if (auto* const object = std::get_if<ObjectType>(&_storage)) {
			if (_objCache != object->end() && _objCache->second.get() == node) {
				auto nextit = std::reverse_iterator<ObjectType::iterator>(_objCache);
				if (++nextit == object->rend()) {
					return nullptr;
				}
				_objCache = nextit.base();
				return nextit->second.get();
			}

			for (auto it = object->rbegin(); it != object->rend(); ++it) {
				if (it->second.get() == node) {
					_objCache = it.base();
					std::reverse_iterator<ObjectType::iterator> nextit = it;
					if (++nextit == object->rend()) {
						return nullptr;
					}
					_objCache = nextit.base();
					return nextit->second.get();
				}
			}

			return nullptr;
		}

		if (auto* const arr = std::get_if<ArrayType>(&_storage)) {
			if (_arrCache != arr->end() && _arrCache->get() == node) {
				auto nextit = std::reverse_iterator<ArrayType::iterator>(_arrCache);
				if (++nextit == arr->rend()) {
					return nullptr;
				}
				_arrCache = nextit.base();
				return nextit->get();
			}

			for (auto it = arr->rbegin(); it != arr->rend(); ++it) {
				if (it->get() == node) {
					_arrCache = it.base();
					std::reverse_iterator<ArrayType::iterator> nextit = it;
					if (++nextit == arr->rend()) {
						return nullptr;
					}
					_arrCache = nextit.base();
					return nextit->get();
				}
			}

			return nullptr;
		}

		return nullptr;
	}

	Config::Detail::Node* Config::Detail::Node::Create(std::string_view key)
	{
		if (auto* const object = std::get_if<ObjectType>(&_storage)) {
			const auto it = object->emplace(key, std::make_unique<Config::Detail::Node>());
			return it->second.get();
		}

		return nullptr;
	}

	plg::string Config::Detail::Node::ToJsonString() const
	{
		plg::string json;

		if (std::get_if<NullType>(&_storage)) {
			json = "null";
		}
		else if (const bool* const boolValue = std::get_if<BoolType>(&_storage)) {
			json = *boolValue ? "true" : "false";
		}
		else if (const int64_t* const numberValue = std::get_if<NumberType>(&_storage)) {
			json = std::to_string(*numberValue);
		}
		else if (const double* const floatValue = std::get_if<FloatType>(&_storage)) {
			json = std::to_string(*floatValue);
		}
		else if (const plg::string* const stringValue = std::get_if<StringType>(&_storage)) {
			json += '\"';
			json += *stringValue;
			json += '\"';
		}

		return json;
	}

	std::unique_ptr<Config> Config::Detail::MakeConfig()
	{
		return std::unique_ptr<Config>(new Config(std::unique_ptr<Detail>(new Detail())));
	}

	Config::Detail::Detail() = default;

	Config::NodeType Config::Detail::GetType() const
	{
		return GetCurrent().GetType();
	}

	bool Config::Detail::IsNull() const
	{
		return GetType() == NodeType::Null;
	}

	bool Config::Detail::IsBool() const
	{
		return GetType() == NodeType::Bool;
	}

	bool Config::Detail::IsInt() const
	{
		return GetType() == NodeType::Int;
	}

	bool Config::Detail::IsFloat() const
	{
		return GetType() == NodeType::Float;
	}

	bool Config::Detail::IsString() const
	{
		return GetType() == NodeType::String;
	}

	bool Config::Detail::IsObject() const
	{
		return GetType() == NodeType::Object;
	}

	bool Config::Detail::IsArray() const
	{
		return GetType() == NodeType::Array;
	}

	void Config::Detail::SetObject()
	{
		GetCurrent().SetObject();
	}

	void Config::Detail::SetArray()
	{
		GetCurrent().SetArray();
	}

	bool Config::Detail::JumpFirst()
	{
		Node* const node = GetCurrent().First();
		if (node) {
			_track.push_back(node);
			return true;
		}
		return false;
	}

	bool Config::Detail::JumpLast()
	{
		Node* const node = GetCurrent().Last();
		if (node) {
			_track.push_back(node);
			return true;
		}
		return false;
	}

	bool Config::Detail::JumpNext()
	{
		Node* const parent = GetCurrentParent();
		if (!parent) {
			return false;
		}

		Node* const next = parent->Next(_track.back());
		if (!next) {
			return false;
		}

		_track.pop_back();
		_track.push_back(next);

		return true;
	}
	
	bool Config::Detail::JumpPrev()
	{
		Node* const parent = GetCurrentParent();
		if (!parent) {
			return false;
		}

		Node* const prev = parent->Prev(_track.back());
		if (!prev) {
			return false;
		}

		_track.pop_back();
		_track.push_back(prev);

		return true;
	}

	bool Config::Detail::JumpKey(std::string_view key, bool create)
	{
		auto& top = GetCurrent();
		auto* node = top.At(key);
		if (!node && create) {
			node = top.Create(key);
		}
		if (node) {
			_track.push_back(node);
			return true;
		}
		return false;
	}

	bool Config::Detail::JumpBack()
	{
		if (_track.empty()) {
			return false;
		}
		_track.pop_back();
		return true;
	}

	void Config::Detail::JumpRoot()
	{
		_track = {};
	}

	plg::string Config::Detail::NodeToJsonString() const
	{
		return GetCurrent().ToJsonString();
	}

	plg::string Config::Detail::RootToJsonString() const
	{
		return _root->ToJsonString();
	}

	Config::Detail::Node& Config::Detail::GetCurrent() const
	{
		if (!_track.empty()) {
			return *(_track.back());
		}

		return *_root;
	}

	Config::Detail::Node* Config::Detail::GetCurrentParent() const
	{
		auto it = _track.crbegin();
		if (it == _track.crend()) {
			return nullptr;
		}
		if (--it == _track.crend()) {
			return _root.get();
		}
		return *it;
	}
	

	Config::NodeType Config::GetType() const
	{
		return _detail->GetType();
	}

	bool Config::IsNull() const
	{
		return _detail->IsNull();
	}

	bool Config::IsBool() const
	{
		return _detail->IsBool();
	}

	bool Config::IsInt() const
	{
		return _detail->IsInt();
	}

	bool Config::IsFloat() const
	{
		return _detail->IsFloat();
	}

	bool Config::IsString() const
	{
		return _detail->IsString();
	}

	bool Config::IsObject() const
	{
		return _detail->IsObject();
	}

	bool Config::IsArray() const
	{
		return _detail->IsArray();
	}

	void Config::SetNull()
	{
		_detail->Set(nullptr);
	}

	void Config::Set(bool value)
	{
		_detail->Set(value);
	}

	void Config::Set(int32_t value)
	{
		_detail->Set(static_cast<int64_t>(value));
	}

	void Config::Set(int64_t value)
	{
		_detail->Set(value);
	}

	void Config::Set(float value)
	{
		_detail->Set(static_cast<double>(value));
	}

	void Config::Set(double value)
	{
		_detail->Set(value);
	}

	void Config::Set(plg::string value)
	{
		_detail->Set(std::move(value));
	}

	void Config::SetObject()
	{
		_detail->SetObject();
	}

	void Config::SetArray()
	{
		_detail->SetArray();
	}

	bool Config::JumpFirst()
	{
		return _detail->JumpFirst();
	}

	bool Config::JumpLast()
	{
		return _detail->JumpLast();
	}

	bool Config::JumpNext()
	{
		return _detail->JumpNext();
	}

	bool Config::JumpPrev()
	{
		return _detail->JumpPrev();
	}

	bool Config::JumpKey(std::string_view key, bool create)
	{
		return _detail->JumpKey(key, create);
	}

	bool Config::JumpBack()
	{
		return _detail->JumpBack();
	}

	void Config::JumpRoot()
	{
		_detail->JumpRoot();
	}


	plg::string Config::NodeToJsonString() const
	{
		return _detail->NodeToJsonString();
	}

	plg::string Config::RootToJsonString() const
	{
		return _detail->RootToJsonString();
	}

	Config::Config(std::unique_ptr<Config::Detail> detail)
		: _detail(std::move(detail))
	{
	}

	Config::~Config() = default;
}
