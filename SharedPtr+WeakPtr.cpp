#include <iostream>
struct BaseControlBlock {
    size_t count_shared;
    size_t count_weak;
    BaseControlBlock(size_t shared, size_t weak) : count_shared(shared), count_weak(weak) {}

    virtual void delete_T() {}
    virtual void delete_all() {}
    virtual void* get() { return nullptr; }
    virtual ~BaseControlBlock() = default;
};

template <typename T, typename Deleter = std::default_delete<T>,
          typename Allocator = std::allocator<T>>
struct ControlBlock : BaseControlBlock {
    T* ptr;
    Deleter deleter;
    Allocator allocator;
    using BlockAlloc =
            typename std::allocator_traits<Allocator>:: template rebind_alloc<ControlBlock>;
    using BlockTraits = std::allocator_traits<BlockAlloc>;

    ControlBlock(size_t shared, size_t weak, T* ptr, Deleter d, Allocator alloc)
            : BaseControlBlock(shared, weak), ptr(ptr), deleter(d), allocator(alloc) {}

    virtual void* get() override {
      return static_cast<void*>(ptr);
    }

    virtual void delete_T() override {
      deleter(ptr);
    }

    virtual void delete_all() override {
      this->~ControlBlock();
      BlockAlloc alloc1(allocator);
      BlockTraits::deallocate(alloc1, this, 1);
    }
    //this->~ControlBlock()
    //deallocate(controlblock)
};

// new(ptr) CB(args);

template <typename T, typename Allocator = std::allocator<T>>
struct MakeControlBlock : BaseControlBlock {
    T element;
    using BlockAlloc =
            typename std::allocator_traits<Allocator>:: template rebind_alloc<MakeControlBlock>;
    using BlockTraits = std::allocator_traits<BlockAlloc>;

    using TAlloc = typename std::allocator_traits<Allocator>:: template rebind_alloc<T>;
    using TTraits = std::allocator_traits<TAlloc>;
    TAlloc allocator;

    template<typename... Args>
    MakeControlBlock(size_t shared, size_t weak, Allocator alloc, Args&&... args)
            : BaseControlBlock(shared, weak), element(std::forward<Args>(args)...),
              allocator(alloc) {}

    virtual void* get() override {
      return static_cast<void*>(&element);
    }

    virtual void delete_T() override {
      TTraits::destroy(allocator, &(this->element));
      // destruct(T)
    }

    virtual void delete_all() override {
      BlockAlloc alloc1(allocator);
      BlockTraits::deallocate(alloc1, this, 1);
    }
    //deallocate(controlblock)
};

// SmthTraits::construct(alloc, ptr, agrs...);
template <typename T>
class EnableSharedFromThis;

template <typename T>
class SharedPtr {
    BaseControlBlock* control_block_;
    void* aliasing_ptr_ = nullptr;

    template <typename>
    friend class SharedPtr;

    template <typename>
    friend class WeakPtr;

    template <typename U, typename... Args>
    friend SharedPtr<U> make_shared(Args&&... args);

public:
    SharedPtr(BaseControlBlock* cb): control_block_(cb), aliasing_ptr_(control_block_->get()) {}

    template<typename Y, typename Deleter = std::default_delete<T>,
             typename Allocator = std::allocator<T>>
    SharedPtr(Y* ptr, Deleter d = std::default_delete<T>(),
              Allocator alloc = std::allocator<T>()) {
      using BlockAlloc =
              typename std::allocator_traits<Allocator>::
                                       template rebind_alloc<ControlBlock<T, Deleter, Allocator>>;
      using BlockTraits = std::allocator_traits<BlockAlloc>;
      BlockAlloc alloc1(alloc);
      ControlBlock<T, Deleter, Allocator>* new_block = BlockTraits::allocate(alloc1, 1);
      new(new_block) ControlBlock<T, Deleter, Allocator>(1, 0, ptr, d, alloc);
      control_block_ = new_block;
      if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
        ptr->wp_ = *this;
      }
      aliasing_ptr_ = ptr;
    }

//    SharedPtr(T* ptr): SharedPtr(ptr, std::default_delete<T>(), std::allocator<T>()) {}
    SharedPtr(): control_block_(nullptr) {}

    SharedPtr(const SharedPtr& r) {
      if (r.control_block_ == nullptr) {
        control_block_ = nullptr;
        return;
      }
      control_block_ = r.control_block_;
      control_block_->count_shared++;
      aliasing_ptr_ = r.aliasing_ptr_;
    }

    template<typename Y>
    SharedPtr(const SharedPtr<Y>& r, T* ptr = nullptr) {
      aliasing_ptr_ = ptr;
      control_block_ = r.control_block_;
      if (control_block_ != nullptr) {
        control_block_->count_shared++;
      }
    }

    SharedPtr(SharedPtr&& r) {
      control_block_ = r.control_block_;
      aliasing_ptr_ = r.aliasing_ptr_;
      r.control_block_ = nullptr;
    }

    template<typename Y>
    SharedPtr(SharedPtr<Y>&& r) {
      control_block_ = r.control_block_;
      aliasing_ptr_ = r.aliasing_ptr_;
      r.control_block_ = nullptr;
    }

    template<typename Y>
    void swap(SharedPtr<Y>& other) {
      std::swap(control_block_, other.control_block_);
      std::swap(aliasing_ptr_, other.aliasing_ptr_);
    }

    SharedPtr& operator=(const SharedPtr& other) {
      if (control_block_ == other.control_block_) {
        return *this;
      }
      SharedPtr stack(other);
      swap(stack);
      return *this;
    }

    template<typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
      SharedPtr<Y> stack(other);
      swap(stack);
      return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
      if (control_block_ != nullptr) {
        control_block_->count_shared--;
      }
      control_block_ = other.control_block_;
      aliasing_ptr_ = other.aliasing_ptr_;
      other.control_block_ = nullptr;
      other.aliasing_ptr_ = nullptr;
      return *this;
    }

    template<typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
      if (control_block_ != nullptr) {
        control_block_->count_shared--;
      }
      control_block_ = other.control_block_;
      aliasing_ptr_ = other.aliasing_ptr_;
      other.control_block_ = nullptr;
      other.aliasing_ptr_ = nullptr;
      return *this;
    }

    size_t use_count() const {
      if (control_block_ == nullptr) {
        return 0;
      }
      return control_block_->count_shared;
    }

    void reset() {
      if (control_block_ == nullptr) {
        return;
      }
      SharedPtr new_shared;
      new_shared.swap(*this);
    }

    template<typename Y>
    void reset(Y* ptr) {
      SharedPtr<Y> new_shared(ptr);
      new_shared.swap(*this);
    }

    T* get() const {
      return static_cast<T*>(aliasing_ptr_);
    }

    T& operator*() const {
      return *get();
    }

    T* operator->() const {
      return get();
    }
    ~SharedPtr() {
      if (control_block_ == nullptr) {
        return;
      }
      control_block_->count_shared--;
      if (control_block_->count_shared == 0) {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
          if  (control_block_->count_weak == 1) {
            control_block_->delete_T();
            return;
          }
        }
        control_block_->delete_T();
        if (control_block_->count_weak == 0) {
          control_block_->delete_all();
        }
      }
    }
};

template <typename T>
class WeakPtr {
    BaseControlBlock* control_block_;

    template <typename>
    friend class WeakPtr;

public:
    WeakPtr(): control_block_(nullptr) {}

    template<typename Y>
    WeakPtr(const SharedPtr<Y>& sp) {
      control_block_ = sp.control_block_;
      control_block_->count_weak++;
    }

    WeakPtr(const WeakPtr<T>& sp) {
      control_block_ = sp.control_block_;
      if (control_block_ != nullptr) {
        control_block_->count_weak++;
      }
    }

    template<typename Y>
    WeakPtr(const WeakPtr<Y>& sp) {
      control_block_ = sp.control_block_;
      if (control_block_ != nullptr) {
        control_block_->count_weak++;
      }
    }

    WeakPtr(WeakPtr<T>&& sp) {
      control_block_ = sp.control_block_;
      sp.control_block_ = nullptr;
    }

    template<typename Y>
    WeakPtr(WeakPtr<Y>&& sp) {
      control_block_ = sp.control_block_;
      sp.control_block_ = nullptr;
    }

    template<typename Y>
    WeakPtr& operator=(const SharedPtr<Y>& sp) {
      if (control_block_ != nullptr) {
        control_block_->count_weak--;
        if (control_block_->count_shared == 0 and control_block_->count_weak == 0) {
          control_block_->delete_all();
          control_block_ = nullptr;
        }
      }
      control_block_ = sp.control_block_;
      if (control_block_ != nullptr) {
        control_block_->count_weak++;
      }
      return *this;
    }

    template<typename Y>
    void swap(WeakPtr<Y>& other) {
      std::swap(control_block_, other.control_block_);
    }

    WeakPtr& operator=(const WeakPtr& other) {
      if (control_block_ != nullptr) {
        control_block_->count_weak--;
      }
      WeakPtr stack(other);
      swap(stack);
      return *this;
    }

    template<typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
      if (control_block_ != nullptr) {
        control_block_->count_weak--;
      }
      WeakPtr stack(other);
      swap(stack);
      return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
      control_block_ = other.control_block_;
      other.control_block_ = nullptr;
      return *this;
    }

    template<typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
      control_block_ = other.control_block_;
      other.control_block_ = nullptr;
      return *this;
    }
    bool expired() const {
      return use_count() == 0;
    }
    SharedPtr<T> lock() const {
      if (expired()) {
        return SharedPtr<T>();
      }
      control_block_->count_shared++;
      return SharedPtr<T>(control_block_);
    }
    size_t use_count() const {
      if (control_block_ == nullptr) {
        return 0;
      }
      return control_block_->count_shared;
    }

    ~WeakPtr() {
      if (control_block_ == nullptr) {
        return;
      }
      control_block_->count_weak--;
      if (control_block_->count_shared == 0 and control_block_->count_weak == 0) {
        control_block_->delete_all();
      }
    }
};


template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
  auto* p = new MakeControlBlock<T, std::allocator<T>>{1, 0, std::allocator<T>(),
                                                       std::forward<Args>(args)...};
  SharedPtr<T> new_shared(static_cast<BaseControlBlock*>(p));
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    p->element.wp_ = new_shared;
  }
  return new_shared;
}

template <typename T, typename Allocator, typename... Args>
SharedPtr<T> allocateShared(const Allocator& alloc, Args&&... args) {
  using MakeAlloc = typename std::allocator_traits<Allocator>::
                                          template rebind_alloc<MakeControlBlock<T, Allocator>>;
  using MakeTraits = std::allocator_traits<MakeAlloc>;
  MakeAlloc alloc1(alloc);
  MakeControlBlock<T, Allocator>* new_block = MakeTraits::allocate(alloc1, 1);
  MakeTraits::construct(alloc1, new_block, 1, 0, alloc, std::forward<Args>(args)...);
  return SharedPtr<T>(static_cast<BaseControlBlock*>(new_block));
}

template <typename T>
class EnableSharedFromThis {
private:
    WeakPtr<T> wp_;

    friend SharedPtr<T>;

    template <typename Y, typename... Args>
    friend SharedPtr<Y> makeShared(Args&&... args);

protected:
    EnableSharedFromThis() = default;

public:
    SharedPtr<T> shared_from_this() {
      if (wp_.use_count() == 0) {
        throw std::bad_weak_ptr();
      }
      return wp_.lock();
    }
};
