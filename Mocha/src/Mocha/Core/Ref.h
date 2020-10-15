#pragma once

#include <stdint.h>

namespace Mocha {
	
	//////////////////////////////////////////////////////////
	// Ref Counting
	//////////////////////////////////////////////////////////

	class RefCounted
	{
	public:
		void IncRefCount() const 
		{
			m_RefCounted++;
		}

		void DecRefCount() const
		{
			m_RefCounted--;
		}

	private:
		mutable uint32_t m_RefCounted = 0;
	};

	//////////////////////////////////////////////////////////
	// Ref Object
	//////////////////////////////////////////////////////////

	template<typename T>
	class Ref
	{
	public:
		Ref()
			: m_Instance(nullptr) {}
		Ref(std::nullptr_t n)
			: m_Instance(n) {}
		Ref(T* instance)
			: m_Instance(instance) {}

		template<typename T2>
		Ref(const Ref<T2>& other)
		{
			m_Instance = (T*)other.m_Instance;
			IncRef();
		}

		template<typename T2>
		Ref(Ref<T2>&& other)
		{
			m_Instance = (T*)other.m_Instance;
			other.m_Instance = nullptr;
		}

		~Ref()
		{
			DecRef();
		}

		Ref(const Ref<T>& other)
			: m_Instance(other.m_Instance)
		{
			IncRef();
		}

		Ref& operator=(std::nullptr_t)
		{
			DecRef();
			m_Instance = nullptr;
			return *this;
		}

		Ref& operator=(const Ref<T>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(const Ref<T2>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(Ref<T2>&& other)
		{
			DecRef();

			m_Instance = other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}

		operator bool() { return m_Instance != nullptr; }
		operator bool() const { return m_Instance != nullptr; }

		T* operator->() { return m_Instance; }
		const T* operator->() const { return m_Instance; }

		T& operator*() { return *m_Instance; }
		const T& operator*() const { return *m_Instance; }

		T* Raw() { return m_Instance; }
		const T* Raw() const { return m_Instance; }

		template<typename T2>
		Ref<T2> As()
		{
			return Ref<T2>(*this);
		}

		template<typename... Args>
		static Ref<T> Create(Args&&...args)
		{
			return Ref<T>(new T(std::forward<Args>(args)...));
		}

	private:
		void IncRef() const
		{
			if (m_Instance)
				m_Instance->IncRefCount();
		}

		void DecRef() const
		{
			if (m_Instance)
			{
				m_Instance->DecRefCount();
				if (m_Instance->GetRefCount() == 0)
					delete m_Instance;
			}
		}

	private:

		T* m_Instance;
	};

}