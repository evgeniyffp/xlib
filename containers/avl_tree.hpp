#pragma once

namespace xlib::container::detail {
    template <
      typename Key,
      bool isHaveValue,
      typename T,
      class Compare,
      class Allocator
    >
    class avl_tree {
    public:
      using key_type = Key;
      using mapped_type = T;
      using value_type = std::conditional_t<isHaveValue,
          std::pair<const key_type, mapped_type>,
          const key_type
      >;
      using size_type = std::size_t;

    private:
      using ATR = std::allocator_traits<Allocator>;

#define toNode(x) (static_cast<Node*>(x))

      struct BaseNode {
        BaseNode* left;
        BaseNode* right;

        BaseNode(BaseNode* left, BaseNode* right) : left(left), right(right) {}
        virtual ~BaseNode() = default;
      };

      struct Node : BaseNode {
        BaseNode* up;
        value_type value;

        int64_t height;

        template <typename ValueType>
        Node(BaseNode* left, BaseNode* right, BaseNode* up, ValueType&& value, int64_t height = 0)
              : BaseNode(left, right)
              , up(up)
              , value(std::forward<ValueType>(value))
              , height(height) {}
        ~Node() override = default;
      };

    public:
      class iterator {
        friend class avl_tree<Key, isHaveValue, T, Compare, Allocator>;
      private:
        BaseNode* _ptr;
        BaseNode* _end;

      public:
        iterator(BaseNode* ptr, BaseNode* end) : _ptr(ptr), _end(end) {}

        value_type& operator*() {
          return toNode(_ptr)->value;
        }

        iterator& operator++() {
          if (_ptr == _end) {
            _ptr = _ptr->left;
          }
          else if (_ptr->right == _end) {
            _ptr = _ptr->right;
          }
          else {
            if (_ptr->right == nullptr) {
              BaseNode* old;
              do {
                old = _ptr;
                _ptr = toNode(_ptr)->up;
              } while(_ptr->right == old);
            }
            else {
              _ptr = _ptr->right;
              while (_ptr->left != nullptr) {
                _ptr = _ptr->left;
              }
            }
          }
          return *this;
        }

        bool operator==(const iterator& other) const {
          return _ptr == other._ptr;
        }

        bool operator!=(const iterator& other) const {
          return !(*this == other);
        }

      };

    private:
      using NodeAllocator = ATR::template rebind_alloc<Node>;
      NodeAllocator _NodeAllocator;
      using ATR_Node = std::allocator_traits<NodeAllocator>;

      using BaseNodeAllocator = ATR::template rebind_alloc<BaseNode>;
      BaseNodeAllocator _BaseNodeAllocator;
      using ATR_BaseNode = std::allocator_traits<BaseNodeAllocator>;


      BaseNode* _end = nullptr;
      Node* _root = nullptr;

      size_t _size = 0;

      Compare _Compare;

      bool _try_init() {
        if (_end != nullptr)
          return false;

        _end = ATR_BaseNode::allocate(_BaseNodeAllocator, 1);

        return true;
      }

      int64_t _getHeight(BaseNode* node) {
        return (node == nullptr || node == _end) ? -1 : toNode(node)->height;
      }

      Node* _updateh_and_try_stable(Node* node) {
        for (Node* tmp = toNode(node->up); tmp != nullptr; tmp = toNode(tmp->up)) {
          tmp->height = std::max(_getHeight(tmp->left), _getHeight(tmp->right)) + 1;
        }

        for (Node* tmp = toNode(node->up); tmp != nullptr; tmp = toNode(tmp->up)) {
          if (std::abs(_getHeight(tmp->left) - _getHeight(tmp->right)) >= 2) {
            std::cout << "xlib::detail::avl_tree::_updateh_and_try_stable(): require stable. \n";
            break;
          }
        }

        // TODO stable tree

        return node;
      }

      void _delete_el(Node* node) {
        if (node != nullptr) {
          _delete_el(toNode(node->left));
          _delete_el(toNode(node->right));
          ATR_Node::destroy(_NodeAllocator, node);
          ATR_Node::deallocate(_NodeAllocator, node, 1);
        }
      }

      Key _toKey(const value_type& value) {
        if constexpr (isHaveValue) {
          return value.first;
        }
        else {
          return value;
        }
      }

    public:
      iterator begin() { return { (_end != nullptr) ? _end->left : _end, _end}; }
      iterator end() { return {_end, _end}; }

    public:
      avl_tree() = default;
      ~avl_tree() {
        if (_end != nullptr) {
          if (_end->left != nullptr && _end->right != nullptr) {
            _end->left->left = _end->right->right = nullptr;
            _end->left = _end->right = nullptr;
          }
          ATR_BaseNode::deallocate(_BaseNodeAllocator, _end, 1);
        }

        _delete_el(toNode(_root));
      }

      template <typename ValueType = value_type>
      std::pair<iterator, bool> insert(ValueType&& value) {
        _try_init();

        if (_root == nullptr) {
          _root = ATR_Node::allocate(_NodeAllocator, 1);
          ATR_Node::construct(_NodeAllocator, _root, _end, _end, nullptr, std::forward<ValueType>(value));

          _end->left = _root;
          _end->right = _root;

          ++_size;
          return {{_root, _end}, true};
        }
        else {
          Node* tmp = _root;
          while (true) {
            if (_Compare(_toKey(value), _toKey(tmp->value))) {
              bool is_end = false;
              if (tmp->left == _end) {
                is_end = true;
                tmp->left = nullptr;
              }

              if (tmp->left == nullptr) {
                tmp->left = ATR_Node::allocate(_NodeAllocator, 1);
                ATR_Node::construct(
                    _NodeAllocator, toNode(tmp->left),
                    (is_end ? _end : nullptr), nullptr, tmp, std::forward<ValueType>(value)
                );
                tmp = toNode(tmp->left);

                if (is_end)
                  _end->left = tmp;

                ++_size;

                return {{_updateh_and_try_stable(tmp), _end}, true};
              }
              else {
                tmp = toNode(tmp->left);
              }
            }
            else if (_Compare(_toKey(tmp->value), _toKey(value))) {
              bool is_end = false;
              if (tmp->right == _end) {
                is_end = true;
                tmp->right = nullptr;
              }

              if (tmp->right == nullptr) {
                tmp->right = ATR_Node::allocate(_NodeAllocator, 1);
                ATR_Node::construct(
                    _NodeAllocator, toNode(tmp->right),
                    nullptr, (is_end ? _end : nullptr), tmp, std::forward<ValueType>(value)
                );
                tmp = toNode(tmp->right);

                if (is_end)
                  _end->right = tmp;

                ++_size;

                return {{_updateh_and_try_stable(tmp), _end}, true};
              }
              else {
                tmp = toNode(tmp->right);
              }
            }
            else {
              return {{tmp, _end}, false};
            }
          }
        }
      }

      T& operator[](const Key& key) requires isHaveValue
      {
        auto [it, _] = insert({key, {}});
        return (*it).second;
      }

      size_t size() { return _size; }

      void __print_tree() {
        std::cout << "xlib::avl_tree::__print_tree()\n";
        __print_tree(_root);
        std::cout << "tree has _end " << _end << " node.\n\n";
      }

      void __print_tree(Node* node, int tabs = 0) {
        if (node == nullptr || node == _end)
          return;
        std::cout << std::string(tabs, '\t') << "node " << node << " has left " << node->left << ", right " << node->right << ", up " << node->up << " nodes and height = " << node->height << ".\n";
        __print_tree(toNode(node->left), tabs + 1);
        __print_tree(toNode(node->right), tabs + 1);
      }
    };
}

namespace xlib::container {
  template <
      typename Key,
      typename T,
      class Compare = std::less<Key>,
      class Allocator = std::allocator<std::pair<const Key, T>>
  >
  using avl_tree = detail::avl_tree<Key, true, T, Compare, Allocator>;

  namespace detail {
    struct dont_have_value {};
  }

  template <
      typename Key,
      class Compare = std::less<Key>,
      class Allocator = std::allocator<const Key>
  >
  using avl_tree_without_value = detail::avl_tree<Key, false, detail::dont_have_value, Compare, Allocator>;
}

