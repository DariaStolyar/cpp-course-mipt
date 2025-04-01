#include <algorithm>
#include <iostream>
#include <memory>

template <size_t N>
class StackStorage {
    char memory[N];
    void* top;
    size_t size_stack = N;
public:
    StackStorage(): memory(), top(memory), size_stack(N) {};

    StackStorage(const StackStorage&) = delete;

    template<typename T>
    T* allocate(size_t count) {
      if (std::align(alignof(T), count * sizeof(T), top, size_stack)) {
        T* oldtop = static_cast<T*>(top);
        size_stack -= count * sizeof(T);
        top = static_cast<void*>(static_cast<char*>(top) + count * sizeof(T));
        return oldtop;
      }
      return nullptr;
    }
};

template <typename T, size_t N>
class StackAllocator {
public:
    StackStorage<N>* stack_memory;
    using value_type = T;
    StackAllocator(StackStorage<N>& stack): stack_memory(&stack) {};

    template<typename U>
    StackAllocator(const StackAllocator<U, N>& stack): stack_memory(stack.stack_memory) {};

    T* allocate(size_t count) {
      T* allocated = stack_memory->template allocate<T>(count);

      if (allocated != nullptr) {
        return allocated;
      } else {
        throw std::bad_alloc();
      }
    }

    StackAllocator& operator=(StackAllocator other) {
      stack_memory = other.stack_memory;
      return *this;
    }

    template<typename U>
    bool operator==(const StackAllocator<U, N>& other) const {
      return stack_memory == other.stack_memory;
    }

    template<typename U>
    bool operator!=(const StackAllocator<U, N>& other) const {
      return stack_memory != other.stack_memory;
    }

    void deallocate(T*, size_t) {}

    template<typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    ~StackAllocator() = default;
};

template <typename T, typename Allocator = std::allocator<T>>
class List {
private:

    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;
    };

    struct Node: BaseNode {
        T value;
        Node() = default;
        Node(const T& element): value(element) {}
        ~Node() = default;
    };

    BaseNode fakeNode;
    size_t size_;

    using NodeAlloc = std::allocator_traits<Allocator>:: template rebind_alloc<Node>;
    using NodeTraits = std::allocator_traits<NodeAlloc>;
    [[no_unique_address]] NodeAlloc alloc;

    template <typename... Args>
    List(bool, size_t count, const Allocator& allocator,  const Args&... args):
                             fakeNode{&fakeNode, &fakeNode}, size_(count), alloc(allocator) {
      size_t i = 0;
      bool is_allocated = false;
      Node* node = static_cast<Node*>(&fakeNode);
      Node* new_node = nullptr;

      try {
        for (; i < count; ++i) {
          is_allocated = false;
          new_node = NodeTraits::allocate(alloc, 1);
          is_allocated = true;

          NodeTraits::construct(alloc, new_node, args...);
          node->next = new_node;
          new_node->prev = node;
          new_node->next = nullptr;

          if (i + 1 == count) {
            new_node->next = &fakeNode;
            fakeNode.prev = new_node;
          }
          node = new_node;
        }
      } catch(...) {
        if (is_allocated) {
          NodeTraits::deallocate(alloc, new_node, 1);
        }

        if (i == 0) {
          return;
        }
        for (size_t j = 0; j < i; ++j) {
          BaseNode* prev_node = node->prev;
          NodeTraits::destroy(alloc, node);
          NodeTraits::deallocate(alloc, node, 1);

          if (i + 1 != j) {
            node = static_cast<Node*>(prev_node);
          }
        }
        throw;
      }
    }

public:
    List(): fakeNode{&fakeNode, &fakeNode}, size_(0), alloc() {}

    List(size_t count): List(true, count, std::allocator<T>()) {}

    List(size_t count, const T& number): List(true, count, std::allocator<T>(), number) {}

    List(const Allocator& allocator): fakeNode{&fakeNode, &fakeNode}, size_(0), alloc(allocator) {}

    List(size_t count, const Allocator& allocator): List(true, count, allocator) {}

    List(size_t count, const T& number, const Allocator& allocator): List(true, count, allocator, number) {}

    NodeAlloc get_allocator() const {
      return alloc;
    }

    List(const List& list): size_(list.size_), alloc(std::allocator_traits<NodeAlloc>::select_on_container_copy_construction(list.alloc)) {
      fakeNode = {&fakeNode, &fakeNode};
      size_t i = 0;
      bool is_allocated = false;

      const_iterator iter = list.begin();
      Node* node = static_cast<Node*>(&fakeNode);
      Node* new_node = nullptr;
      try {
        for (; i < size_; ++i) {
          is_allocated = false;
          new_node = NodeTraits::allocate(alloc, 1);
          is_allocated = true;

          NodeTraits::construct(alloc, new_node, *iter);
          node->next = new_node;
          new_node->prev = node;
          new_node->next = nullptr;

          if (i + 1 == size_) {
            new_node->next = &fakeNode;
            fakeNode.prev = new_node;
            continue;
          }
          node = new_node;
          ++iter;
        }
      } catch(...) {
        if (is_allocated) {
          NodeTraits::deallocate(alloc, new_node, 1);
        }

        if (i == 0) {
          return;
        }

        for (size_t j = 0; j < i; ++j) {
          BaseNode* prev_node = node->prev;
          NodeTraits::destroy(alloc, node);
          NodeTraits::deallocate(alloc, node, 1);

          if (i + 1 != j) {
            node = static_cast<Node*>(prev_node);
          }
        }
        throw;
      }
    }

    void swap(List& other) {
      std::swap(fakeNode.prev, other.fakeNode.prev);
      std::swap(fakeNode.next, other.fakeNode.next);
      std::swap(fakeNode.prev->next, other.fakeNode.prev->next);
      std::swap(fakeNode.next->prev, other.fakeNode.next->prev);
      std::swap(size_, other.size_);
    }

    List& operator=(const List& other) {
      if (this == &other) {
        return *this;
      }

      List stack(other);
      swap(stack);

      if (std::allocator_traits<NodeAlloc>::propagate_on_container_copy_assignment::value) {
        alloc = other.alloc;
      }
      return *this;
    }

    void push_back(const T& element) {
      insert(end(), element);
    }

    void push_front(const T& element) {
      insert(begin(), element);
    }

    void pop_front() {
      erase(begin());
    }

    void pop_back() {
      erase(--end());
    }

    ~List() {
      while (size_ != 0) {
        pop_front();
      }
    }

    size_t size() const {
      return size_;
    }

    template <bool isConst>
    struct common_iterator {
    private:
        using base_pointer = typename std::conditional<isConst, const BaseNode*, BaseNode*>::type;
        using node_pointer = std::conditional<isConst, const Node*, Node*>::type;

        base_pointer ptr;

        common_iterator(base_pointer ptr): ptr(ptr) {};

        node_pointer findNode() const {
          return static_cast<node_pointer>(ptr);
        }
        friend List;

    public:
        using difference_type = int;
        using value_type = T;
        using reference = typename std::conditional<isConst, const T&, T&>::type;
        using pointer = typename std::conditional<isConst, const T*, T*>::type;
        using iterator_category = std::bidirectional_iterator_tag;

        operator common_iterator<true>() const {
          return common_iterator<true>(ptr);
        };

        reference operator*() const {
          return static_cast<node_pointer>(ptr)->value;
        }

        pointer operator->() const {
          return &(operator*());
        }

        common_iterator& operator++() {
          ptr = ptr->next;
          return *this;
        }

        common_iterator& operator--() {
          ptr = ptr->prev;
          return *this;
        }

        common_iterator operator++(int) {
          base_pointer prev_ptr = ptr;
          ++*this;
          return prev_ptr;
        }

        common_iterator operator--(int) {
          base_pointer prev_ptr = ptr;
          --*this;
          return prev_ptr;
        }

        bool operator==(const common_iterator& other) const = default;
    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<common_iterator<false>>;
    using const_reverse_iterator = std::reverse_iterator<common_iterator<true>>;

    iterator begin() {
      return iterator(fakeNode.next);
    }
    const_iterator begin() const {
      return const_iterator(fakeNode.next);
    }

    iterator end() {
      return iterator(&fakeNode);
    }
    const_iterator end() const {
      return const_iterator(&fakeNode);
    }

    const_iterator cbegin() const {
      return const_iterator(begin());
    }
    const_iterator cend() const {
      return const_iterator(end());
    }

    reverse_iterator rbegin() {
      return reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const {
      return const_reverse_iterator(cend());
    }

    reverse_iterator rend() {
      return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const {
      return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator crbegin() const {
      return const_reverse_iterator(cend());
    }
    const_reverse_iterator crend() const {
      return const_reverse_iterator(cbegin());
    }

    void insert(const_iterator iter, const T& element) {
      Node* node = const_cast<Node*>(iter.findNode());
      bool is_allocated= false;
      Node* new_node;

      try {
        new_node = NodeTraits::allocate(alloc, 1);
        is_allocated= true;
        NodeTraits::construct(alloc, new_node, element);

        node->prev->next = new_node;
        new_node->prev = node->prev;
        node->prev = new_node;
        new_node->next = node;
        size_++;
      } catch(...) {
        if (is_allocated) {
          NodeTraits::deallocate(alloc, new_node, 1);
        }
        throw;
      }
    }
    void erase(const_iterator iter) {
      Node* node = const_cast<Node*>(iter.findNode());
      node->prev->next = node->next;
      node->next->prev = node->prev;

      NodeTraits::destroy(alloc, node);
      NodeTraits::deallocate(alloc, node, 1);
      size_--;
    }
};
