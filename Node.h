#pragma once
#include <cassert>
#include <array>
#include <algorithm>
#include <vector>
#include <stack>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <utility>
#include <functional>
#include <string>
#include <Windows.h>
namespace ShaderNodeEditor {
    struct Id
    {
        size_t id;

        inline bool is_valid() const { return id > 0; }

        inline void invalidate() { id = invalid_index; }

        inline operator size_t() const
        {
            assert(is_valid());
            return id;
        }

        inline Id& operator=(size_t i)
        {
            id = i;
            return *this;
        }

        inline bool operator==(size_t i) const { return id == i; }

        Id() : id(invalid_index) {}

    private:
        static const size_t invalid_index = 0;
    };
    struct OutputNode
    {
        size_t in;
        size_t out;
    };
    enum PinValueType {
        Demical_Float,
        Demical_INT,
        Demical_UINT,
        Demical_BOOL,
        Float,
        Vector2,
        Vector3,
        Vector4,
        Matrix2,
        Matrix3,
        Matrix4,
        INT,
        UINT,
        BOOL,
    };
    struct PinValue {
        float fVal[16];
    };
    struct NodeParam {
        NodeParam(size_t _id) :id(_id) {}
        size_t id;
        PinValue value;
        //operator const size_t() const{ return id; }
    };
    struct NodeId
    {
        size_t op;
        std::vector<NodeParam> params;
    };

    template<class T>
    T clamp(T x, T a, T b)
    {
        return std::min(b, std::max(x, a));
    }

    // The type T must be POD
    template<class T, size_t N>
    class StaticVector
    {
    public:
        using Iterator = T *;
        using ConstIterator = const T*;

        StaticVector() : storage_(), size_(0) {}
        ~StaticVector() { size_ = 0; }

        // Element access

        inline T* data() { return storage_; }
        inline const T* data() const { return storage_; }

        inline T& back()
        {
            return const_cast<T&>(static_cast<const StaticVector*>(this)->back());
        }
        inline const T& back() const
        {
            assert(size_ > 0u);
            assert(size_ <= N);
            return storage_[size_ - 1];
        }

        inline T& operator[](const size_t i)
        {
            return const_cast<T&>(
                static_cast<const StaticVector*>(this)->operator[](i));
        }
        inline const T& operator[](const size_t i) const
        {
            assert(i < size_);
            return storage_[i];
        }

        inline Iterator find(const T& t)
        {
            return const_cast<Iterator>(
                static_cast<const StaticVector*>(this)->find(t));
        }
        inline ConstIterator find(const T& t) const
        {
            auto iter = begin();
            while (iter != end())
            {
                if (*iter == t)
                {
                    return iter;
                }
                ++iter;
            }
            return iter;
        }

        // Capacity

        inline bool empty() const { return size_ == 0u; }

        inline size_t size() const { return size_; }

        inline size_t capacity() const { return N; }

        // Modifiers

        inline void push_back(const T& elem)
        {
            assert(size_ < N);
            storage_[size_] = elem;
            ++size_;
        }

        inline void pop_back()
        {
            assert(size_);
            --size_;
        }

        inline void swap_erase(size_t remove_at) { swap_erase(data() + remove_at); }
        inline void swap_erase(Iterator iter)
        {
            assert(size_ > 0u);
            assert(size_t(iter - begin()) < size_);

            if (iter != &back())
            {
                std::swap(*iter, back());
            }

            pop_back();
        }

        inline void clear() { size_ = 0u; }

        // Iterators

        inline Iterator begin() { return data(); }
        inline ConstIterator begin() const { return data(); }

        inline Iterator end() { return storage_ + size_; }
        inline ConstIterator end() const { return storage_ + size_; }

    private:
        T storage_[N];
        size_t size_;
    };

    enum NodeType
    {
        Node_Number,
        Node_NumberExpression,
        Node_Operation,
        Node_Output
    };
    struct NodeInputPinMeta {
        const char* name;
        PinValueType pin;
    };
    struct NodeOutputPinMeta {
        const char* name;
    };
    struct INodeInterpreter {
        INodeInterpreter() = default;
        virtual ~INodeInterpreter() = default;
        INodeInterpreter& operator=(const INodeInterpreter&) = default;
        INodeInterpreter(const INodeInterpreter&) = default;
        virtual std::string Interpret() = 0;
        virtual const NodeInputPinMeta& GetInputMetaInfo(size_t i) const = 0;
        virtual const NodeOutputPinMeta& GetOutputMetaInfo(size_t i) const = 0;
        virtual const char* GetCategory() const = 0;
        virtual bool ForceShowNumber() { return false; }
        //如果有变量名,就单独创建一个变量
        std::string variableName;
        NodeType type;
        PinValue number;//constant variable output用
        NodeId Id;
    };

#define FORCE_SHOW_NUMBER bool ForceShowNumber()override { return true; }
    struct IVariableInterpreter :public INodeInterpreter {
        IVariableInterpreter() = default;
        virtual ~IVariableInterpreter() = default;
        FORCE_SHOW_NUMBER
    };
    struct IConstantInterpreter :public INodeInterpreter {
        IConstantInterpreter() = default;
        virtual ~IConstantInterpreter() = default;
        FORCE_SHOW_NUMBER
    };
    struct IOperationInterpreter :public INodeInterpreter {
        IOperationInterpreter() = default;
        virtual ~IOperationInterpreter() = default;
    };

    struct Edge
    {
        // the from, to variables store the node ids of the nodes contained in the
        // edge.
        size_t from, to;

        Edge(size_t f, size_t t) : from(f), to(t) {}

        // seems like std::unordered_map requires this to be
        // default-constructible...
        Edge() : from(), to() {}

        inline size_t opposite(size_t n) const { return n == from ? to : from; }
    }; class Graph
    {
    public:
        // the graph has a limited number of adjacencies, simplifies memory usage
        using AdjacencyArray = StaticVector<size_t, 4u>;

        using EdgeIterator = std::unordered_map<size_t, Edge>::iterator;
        using ConstEdgeIterator = std::unordered_map<size_t, Edge>::const_iterator;

        Graph()
            : current_id_(0u), nodes_(), edges_from_node_(), edges_to_node_(),
            edges_()
        {
        }

        // Element access

        inline std::shared_ptr<INodeInterpreter> node(const size_t node_id)
        {
            return static_cast<const Graph*>(this)->node(node_id);
        }
        inline const std::shared_ptr<INodeInterpreter>& node(const size_t node_id) const
        {
            assert(nodes_.find(node_id) != nodes_.end());
            return nodes_.at(node_id);
        }

        inline const AdjacencyArray& edges_from_node(const size_t node_id)
        {
            return edges_from_node_[node_id];
        }

        inline const AdjacencyArray& edges_to_node(const size_t node_id)
        {
            return edges_to_node_[node_id];
        }

        inline Edge& edge(const size_t edge_id)
        {
            return const_cast<Edge&>(
                static_cast<const Graph*>(this)->edge(edge_id));
        }
        inline const Edge& edge(const size_t edge_id) const
        {
            assert(edges_.find(edge_id) != edges_.end());
            return edges_.at(edge_id);
        }

        inline EdgeIterator begin_edges() { return edges_.begin(); }
        inline ConstEdgeIterator end_edges() const { return edges_.begin(); }

        inline EdgeIterator end_edges() { return edges_.end(); }
        inline ConstEdgeIterator end_edge() const { return edges_.end(); }

        // Modifiers

        size_t add_node(const std::shared_ptr<INodeInterpreter>& node)
        {
            const size_t id = current_id_++;
            nodes_.insert(std::make_pair(id, node));
            edges_from_node_.insert(std::make_pair(id, AdjacencyArray()));
            edges_to_node_.insert(std::make_pair(id, AdjacencyArray()));
            return id;
        }

        void erase_node(const NodeParam param) {
            return erase_node(param.id);
        }
        void erase_node(const size_t node_id)
        {
            // first, collect all the edges from the adjacency lists
            // since erasing an edge invalidates the adjacency list iterators
            StaticVector<size_t, 6> edges_to_erase;
            for (size_t edge : edges_from_node_[node_id])
            {
                edges_to_erase.push_back(edge);
            }
            for (size_t edge : edges_to_node_[node_id])
            {
                edges_to_erase.push_back(edge);
            }
            for (size_t edge : edges_to_erase)
            {
                erase_edge(edge);
            }
            nodes_.erase(node_id);
            edges_from_node_.erase(node_id);
            edges_to_node_.erase(node_id);
        }

        size_t add_edge(const size_t from, const size_t to)
        {
            const size_t id = current_id_++;
            edges_.insert(std::make_pair(id, Edge(from, to)));
            edges_from_node_[from].push_back(id);
            edges_to_node_[to].push_back(id);
            return id;
        }

        void erase_edge(const size_t edge_id)
        {
            auto edge = edges_.find(edge_id);
            assert(edge != edges_.end());

            {
                auto& edges_from = edges_from_node_[edge->second.from];
                auto iter = edges_from.find(edge_id);
                assert(iter != edges_from.end());
                edges_from.swap_erase(iter);
            }

            {
                auto& edges_to = edges_to_node_[edge->second.to];
                auto iter = edges_to.find(edge_id);
                assert(iter != edges_to.end());
                edges_to.swap_erase(iter);
            }

            edges_.erase(edge);
        }
        void evaluate(const size_t root_node);
        
    private:
        size_t current_id_;
        std::unordered_map<size_t, std::shared_ptr<INodeInterpreter>> nodes_;
        std::unordered_map<size_t, AdjacencyArray> edges_from_node_;
        std::unordered_map<size_t, AdjacencyArray> edges_to_node_;
        std::unordered_map<size_t, Edge> edges_;
    };

}
