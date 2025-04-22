#pragma once
#include <mutex>
#include "Queue.h"


/**
 * Thread-Sa?fe한 Queue
 *
 * Shader Hot Reload를 위한 임시용
 */
template <typename T, typename Allocator = FDefaultAllocator<T>>
class TSafeQueue : public TQueue<T, Allocator>
{
private:
    using Super = TQueue<T, Allocator>;
    mutable std::mutex Mutex;

public:
    TSafeQueue() = default;
    ~TSafeQueue() = default;

    TSafeQueue(const TSafeQueue&) = delete;
    TSafeQueue& operator=(const TSafeQueue&) = delete;

    TSafeQueue(TSafeQueue&&) = delete;
    TSafeQueue& operator=(TSafeQueue&&) = delete;

public:
    /**
     * 큐의 맨 뒤에 새 요소를 추가합니다 (복사).
     *
     * @param Item 추가할 요소
     * @return 성공적으로 추가되었으면 true (std::deque는 일반적으로 예외를 던지지 않는 한 성공)
     */
    bool Enqueue(const typename Super::ElementType& Item)
    {
        std::lock_guard Lock(Mutex);
        Super::GetContainerPrivate().push_back(Item);
        return true; // std::deque::push_back은 성공 시 void 반환, 실패 시 예외. 여기서는 bool 반환 스타일 유지
    }

    /**
     * 큐의 맨 뒤에 새 요소를 추가합니다 (이동).
     *
     * @param Item 추가할 요소 (rvalue 참조)
     * @return 성공적으로 추가되었으면 true
     */
    bool Enqueue(typename Super::ElementType&& Item)
    {
        std::lock_guard Lock(Mutex);
        Super::GetContainerPrivate().push_back(std::move(Item));
        return true;
    }

    /**
     * 큐의 맨 뒤에 새 요소를 직접 생성하여 추가합니다 (emplacement).
     * TArray의 Emplace와 유사하게 작동합니다.
     *
     * @tparam ArgsType 생성자 인자 타입들
     * @param Args 요소 생성에 필요한 인자들
     * @return 성공적으로 추가되었으면 true
     */
    template <typename... ArgsType>
    bool Emplace(ArgsType&&... Args)
    {
        std::lock_guard Lock(Mutex);
        Super::GetContainerPrivate().emplace_back(std::forward<ArgsType>(Args)...);
        return true;
    }

    /**
     * 큐의 맨 앞에서 요소를 제거하고 그 값을 반환합니다.
     * 큐가 비어있으면 false를 반환하고 OutItem은 변경되지 않습니다.
     *
     * @param OutItem 제거된 요소의 값을 저장할 변수 (출력 파라미터)
     * @return 요소가 성공적으로 제거되었으면 true, 큐가 비어있으면 false
     */
    bool Dequeue(typename Super::ElementType& OutItem)
    {
        std::lock_guard Lock(Mutex);
        if (IsEmpty())
        {
            return false;
        }
        // front()는 예외를 던지지 않지만, pop_front()는 비어있을 때 정의되지 않은 동작일 수 있으므로 IsEmpty 체크 필수.
        // std::move를 사용하여 가능하면 이동 시맨틱을 활용합니다.
        OutItem = std::move(Super::GetContainerPrivate().front());
        Super::GetContainerPrivate().pop_front();
        return true;
    }

	/**
	 * 큐의 맨 앞에서 요소를 제거합니다. (값을 반환하지 않음)
	 * 큐가 비어있으면 아무 작업도 하지 않습니다.
	 *
	 * @return 요소를 제거했으면 true, 큐가 비어있으면 false
	 */
	bool Dequeue()
	{
        std::lock_guard Lock(Mutex);
		if (IsEmpty())
		{
			return false;
		}
		Super::GetContainerPrivate().pop_front();
		return true;
	}

    /**
     * 큐의 맨 앞에 있는 요소를 제거하지 않고 그 값을 확인합니다.
     * 큐가 비어있으면 false를 반환하고 OutItem은 변경되지 않습니다.
     *
     * @param OutItem 맨 앞 요소의 값을 저장할 변수 (출력 파라미터)
     * @return 맨 앞 요소를 성공적으로 읽었으면 true, 큐가 비어있으면 false
     */
    bool Peek(typename Super::ElementType& OutItem) const
    {
        std::lock_guard Lock(Mutex);
        if (IsEmpty())
        {
            return false;
        }
        // const 버전이므로 복사합니다.
        OutItem = Super::GetContainerPrivate().front();
        return true;
    }

	/**
	 * 큐의 맨 앞에 있는 요소를 제거하지 않고 포인터를 반환합니다.
	 * 큐가 비어있으면 nullptr을 반환합니다.
	 * 주의: Dequeue나 Empty 호출 시 이 포인터는 유효하지 않게 됩니다.
	 *
	 * @return 맨 앞 요소에 대한 포인터, 큐가 비어있으면 nullptr
	 */
	typename Super::ElementType* Peek()
	{
        std::lock_guard Lock(Mutex);
		if (IsEmpty())
		{
			return nullptr;
		}
		return &Super::GetContainerPrivate().front();
	}

	/**
	 * 큐의 맨 앞에 있는 요소를 제거하지 않고 const 포인터를 반환합니다. (const 버전)
	 * 큐가 비어있으면 nullptr을 반환합니다.
	 *
	 * @return 맨 앞 요소에 대한 const 포인터, 큐가 비어있으면 nullptr
	 */
	const typename Super::ElementType* Peek() const
	{
        std::lock_guard Lock(Mutex);
		if (IsEmpty())
		{
			return nullptr;
		}
		return &Super::GetContainerPrivate().front();
	}

    /** 큐의 맨 앞에서 요소를 제거합니다. (값을 반환하지 않음) */
    bool Pop()
    {
        std::lock_guard Lock(Mutex);
        if (IsEmpty())
        {
            return false;
        }
        Super::GetContainerPrivate().pop_front();
        return true;
    }

public:
    /**
     * 큐가 비어 있는지 확인합니다.
     *
     * @return 큐가 비어있으면 true, 아니면 false
     */
    [[nodiscard]] bool IsEmpty() const
    {
        return Super::GetContainerPrivate().empty();
    }

    /** 큐의 모든 요소를 제거합니다. */
    void Empty()
    {
        std::lock_guard Lock(Mutex);
        Super::GetContainerPrivate().clear();
    }

    /**
     * 큐에 있는 요소의 개수를 반환합니다.
     *
     * @return 큐의 요소 개수
     */
    typename Super::SizeType Num() const
    {
        return Super::GetContainerPrivate().size();
    }
};
