#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace MTL_A
{

template<typename Key, typename Value, typename Hash = std::hash<Key>>
class MyUnoredredMap
{
private:
    struct Node
    {
        Key key;
        Value value;
        Node* next;

        Node(const Key& k, const Value& v)
            : key(k), value(v), next(nullptr) {}
    };

    std::vector<Node*> buckets_;
    std::size_t size_ = 0;
    Hash hasher_ = Hash{};

    std::size_t bucket_index(const Key& key) const
    {
        return hasher_(key) % buckets_.size();
    }

    Node* find_node(const Key& key)
    {
        Node* cur = buckets_[bucket_index(key)];
        while (cur)
        {
            if (cur->key == key)
                return cur;
            cur = cur->next;
        }
        return nullptr;
    }

    const Node* find_node(const Key& key) const
    {
        const Node* cur = buckets_[bucket_index(key)];
        while (cur)
        {
            if (cur->key == key)
                return cur;
            cur = cur->next;
        }
        return nullptr;
    }

    void ensure_capacity()
    {
        if (buckets_.empty())
            buckets_.resize(8, nullptr);

        if ((size_ + 1) * 4 > buckets_.size() * 3)
            rehash(buckets_.size() * 2);
    }

    void copy_from(const MyUnoredredMap& other)
    {
        buckets_.assign(other.buckets_.size(), nullptr);
        size_ = 0;

        for (std::size_t i = 0; i < other.buckets_.size(); ++i)
        {
            const Node* cur = other.buckets_[i];
            while (cur)
            {
                insert(cur->key, cur->value);
                cur = cur->next;
            }
        }
    }

public:
    explicit MyUnoredredMap(std::size_t bucket_count = 8)
        : buckets_(bucket_count == 0 ? 8 : bucket_count, nullptr)
    {
    }

    MyUnoredredMap(const MyUnoredredMap& other)
    {
        copy_from(other);
    }

    MyUnoredredMap& operator=(const MyUnoredredMap& other)
    {
        if (this == &other)
            return *this;

        clear();
        copy_from(other);
        return *this;
    }

    MyUnoredredMap(MyUnoredredMap&& other) noexcept
        : buckets_(std::move(other.buckets_)),
          size_(other.size_),
          hasher_(std::move(other.hasher_))
    {
        other.size_ = 0;
        other.buckets_.clear();
    }

    MyUnoredredMap& operator=(MyUnoredredMap&& other) noexcept
    {
        if (this == &other)
            return *this;

        clear();
        buckets_ = std::move(other.buckets_);
        size_ = other.size_;
        hasher_ = std::move(other.hasher_);

        other.size_ = 0;
        other.buckets_.clear();
        return *this;
    }

    ~MyUnoredredMap()
    {
        clear();
    }

    bool empty() const noexcept
    {
        return size_ == 0;
    }

    std::size_t size() const noexcept
    {
        return size_;
    }

    std::size_t bucket_count() const noexcept
    {
        return buckets_.size();
    }

    void clear() noexcept
    {
        for (std::size_t i = 0; i < buckets_.size(); ++i)
        {
            Node* cur = buckets_[i];
            while (cur)
            {
                Node* next = cur->next;
                delete cur;
                cur = next;
            }
            buckets_[i] = nullptr;
        }
        size_ = 0;
    }

    bool contains(const Key& key) const
    {
        return find_node(key) != nullptr;
    }

    bool insert(const Key& key, const Value& value)
    {
        if (find_node(key))
            return false;

        ensure_capacity();
        const std::size_t index = bucket_index(key);
        Node* node = new Node(key, value);
        node->next = buckets_[index];
        buckets_[index] = node;
        ++size_;
        return true;
    }

    void insert_or_assign(const Key& key, const Value& value)
    {
        Node* node = find_node(key);
        if (node)
        {
            node->value = value;
            return;
        }

        ensure_capacity();
        const std::size_t index = bucket_index(key);
        node = new Node(key, value);
        node->next = buckets_[index];
        buckets_[index] = node;
        ++size_;
    }

    bool erase(const Key& key)
    {
        const std::size_t index = bucket_index(key);
        Node* cur = buckets_[index];
        Node* prev = nullptr;

        while (cur)
        {
            if (cur->key == key)
            {
                if (prev)
                    prev->next = cur->next;
                else
                    buckets_[index] = cur->next;

                delete cur;
                --size_;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    Value& at(const Key& key)
    {
        Node* node = find_node(key);
        if (!node)
            throw std::runtime_error("MyUnoredredMap::at key not found");
        return node->value;
    }

    const Value& at(const Key& key) const
    {
        const Node* node = find_node(key);
        if (!node)
            throw std::runtime_error("MyUnoredredMap::at key not found");
        return node->value;
    }

    Value* find(const Key& key)
    {
        Node* node = find_node(key);
        return node ? &node->value : nullptr;
    }

    const Value* find(const Key& key) const
    {
        const Node* node = find_node(key);
        return node ? &node->value : nullptr;
    }

    Value& operator[](const Key& key)
    {
        Node* node = find_node(key);
        if (node)
            return node->value;

        ensure_capacity();
        const std::size_t index = bucket_index(key);
        node = new Node(key, Value{});
        node->next = buckets_[index];
        buckets_[index] = node;
        ++size_;
        return node->value;
    }

    void rehash(std::size_t new_bucket_count)
    {
        if (new_bucket_count < 8)
            new_bucket_count = 8;

        std::vector<Node*> new_buckets(new_bucket_count, nullptr);

        for (std::size_t i = 0; i < buckets_.size(); ++i)
        {
            Node* cur = buckets_[i];
            while (cur)
            {
                Node* next = cur->next;
                const std::size_t new_index = hasher_(cur->key) % new_bucket_count;
                cur->next = new_buckets[new_index];
                new_buckets[new_index] = cur;
                cur = next;
            }
            buckets_[i] = nullptr;
        }

        buckets_.swap(new_buckets);
    }
};

template<typename Key, typename Value, typename Hash = std::hash<Key>>
using MyUnorderedMap = MyUnoredredMap<Key, Value, Hash>;

}
