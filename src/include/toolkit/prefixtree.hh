#pragma once
#ifndef INCLUDED_lptk_prefixtree_HH
#define INCLUDED_lptk_prefixtree_HH

#include <memory>
#include "toolkit/hashmap.hh"

namespace lptk
{
    template<class K, class V>
    class PrefixTree
    {
    public:
        class handle;

        handle get(const K& key, const handle& start);
        handle insert(const K& key, const V& value, const handle& start);
        handle getRoot() ;
    private:
        struct Node
        {
            typedef HashMap<K, std::unique_ptr<Node>> MapType ;
            MapType m_children;
            V m_value;
        };

        Node m_root;
    };

    template<class K, class V>
    class PrefixTree<K, V>::handle
    {
        friend class PrefixTree<K, V>;
        typedef PrefixTree<K, V>::Node NodeType;
    public:
        handle() : m_node(nullptr) {}
        handle(NodeType* node) : m_node(node) {}

        bool isValid() const { return m_node != nullptr; }

        V& getValue() { return m_node->m_value; }
        const V& getValue() const { return m_node->m_value; }

        void setValue(const V& value) { m_node->m_value = value; }
    private:
        NodeType* m_node;
    };

    
    template<class K, class V>
    typename PrefixTree<K, V>::handle PrefixTree<K, V>::get(const K& key, const handle& start)
    {
        if(!start.isValid())
            return handle();

        auto pair = start.m_node->m_children.getpair(key);
        if(!pair)
            return handle();

        return handle(pair->value.get());
    }

    template<class K, class V>
    typename PrefixTree<K, V>::handle PrefixTree<K, V>::insert(const K& key, const V& value, const handle& start)
    {
        if(!start.isValid())
            return handle();

        auto pair = start.m_node->m_children.getpair(key);
        if(!pair)
            pair = start.m_node->m_children.set(key, make_unique<Node>());
        pair->value->m_value = value;
        return handle(pair->value.get());
    }

    template<class K, class V>
    typename PrefixTree<K, V>::handle PrefixTree<K, V>::getRoot() 
    {
        return handle(&m_root);
    }

}

#endif

