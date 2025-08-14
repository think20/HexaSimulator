#pragma once
template <class Ty>
class TQueue_t
{
public:
	bool Push(Ty);
	bool Pop();
	Ty Front();
	void Clear();
	bool Empty();
	TQueue_t(unsigned int MaxSize) :MaxSize(MaxSize), Size(0), HeadIndex(0), TailIndex(0), Container(new Ty[MaxSize]) {};
	~TQueue_t() { delete this->Container; };
	unsigned int DebugGetSize() { return this->Size; };

protected:
	Ty* Container;
	unsigned int MaxSize, Size, HeadIndex, TailIndex;
};

template <class Ty>
class TVector_t
{
	
public:
	bool Push(Ty);
	bool Pop();
	Ty Front();
	void Clear();
	bool Empty();
	TVector_t(unsigned int MaxSize=1) :MaxSize(MaxSize), Size(0), HeadIndex(0), TailIndex(0), Container(new Ty[MaxSize]),HeadAlter(true),TailAlter(true) {};
	~TVector_t() { delete this->Container; };
	unsigned int DebugGetSize() { return this->Size; };
	
	class Iterator_t
	{
	public:
		Iterator_t(unsigned int i = 0, TVector_t<Ty>* V = nullptr) :Index(i), Vec(V) {};
		Iterator_t(TVector_t<Ty>::Iterator_t& Iter)
		{
			this->Index = Iter.Index;
			this->Vec = Iter.Vec;
		}
		TVector_t<Ty>::Iterator_t& operator++()//left ++i
		{
			Index = Index + 1;
			if (Index == Vec->MaxSize) Index = 0;
			return *this;
		}
		TVector_t<Ty>::Iterator_t& operator++(int)//right i++
		{
			Index = Index + 1;
			if (Index == Vec->MaxSize) Index = 0;
			return *this;
		}
		bool operator==(TVector_t<Ty>::Iterator_t& Iter2)
		{
			return(Index == Iter2.Index);
		}
		Ty* operator->()
		{
			return (&(this->Vec->Container[Index]));
		}
		unsigned int DebugGetIndex()
		{
			return this->Index;
		};
	private:
		unsigned int Index;
		TVector_t<Ty>* Vec;
	};

	TVector_t<Ty>::Iterator_t* LazyBegin;
	TVector_t<Ty>::Iterator_t* LazyEnd;
	TVector_t<Ty>::Iterator_t Begin();
	TVector_t<Ty>::Iterator_t End();

	Ty* Container;
protected:
	unsigned int MaxSize, Size, HeadIndex, TailIndex;
	bool HeadAlter, TailAlter;
};

template <class Ty>
bool TQueue_t<Ty>::Push(Ty Element)
{
	if (this->Size == MaxSize) return false;
	(this->Size)++;

	this->Container[this->TailIndex] = Element;
	(this->TailIndex)++;
	if (this->TailIndex == MaxSize) this->TailIndex = 0;

	return true;
}

template <class Ty>
bool TQueue_t<Ty>::Pop()
{
	if (this->Size == 0) return false;
	(this->Size)--;

	(this->HeadIndex)++;
	if (this->HeadIndex == MaxSize) this->HeadIndex = 0;

	return true;

}

template <class Ty>
Ty TQueue_t<Ty>::Front()
{
	return(this->Container[HeadIndex]);
}

template <class Ty>
void TQueue_t<Ty>::Clear()
{
	this->HeadIndex = 0;
	this->TailIndex = 0;
	this->Size = 0;
}

template <class Ty>
bool TQueue_t<Ty>::Empty()
{
	return(this->Size == 0);
}

template <class Ty>
bool TVector_t<Ty>::Push(Ty Element)
{
	if (this->Size == MaxSize) return false;
	(this->Size)++;

	this->Container[this->TailIndex] = Element;
	(this->TailIndex)++;
	if (this->TailIndex == MaxSize) this->TailIndex = 0;

	this->TailAlter = true;

	return true;
}

template <class Ty>
bool TVector_t<Ty>::Pop()
{
	if (this->Size == 0) return false;
	(this->Size)--;

	(this->HeadIndex)++;
	if (this->HeadIndex == MaxSize) this->HeadIndex = 0;

	this->HeadAlter = true;

	return true;

}

template <class Ty>
Ty TVector_t<Ty>::Front()
{
	return(this->Container[HeadIndex]);
}

template <class Ty>
void TVector_t<Ty>::Clear()
{
	this->HeadIndex = 0;
	this->TailIndex = 0;
	this->Size = 0;

	this->HeadAlter = true;
	this->TailAlter = true;
}

template <class Ty>
bool TVector_t<Ty>::Empty()
{
	return(this->Size == 0);
}

template <class Ty>
TVector_t<Ty>::Iterator_t TVector_t<Ty>::Begin()
{
	/*if (this->HeadAlter)
		this->LazyBegin = new TVector_t<Ty>::Iterator_t(this->HeadIndex, this);
	this->HeadAlter = false;
	return *(this->LazyBegin);*/
	return TVector_t<Ty>::Iterator_t(this->HeadIndex, this);
}

template <class Ty>
TVector_t<Ty>::Iterator_t TVector_t<Ty>::End()
{
	/*if (this->TailAlter)
		this->LazyEnd = new TVector_t<Ty>::Iterator_t(this->TailIndex, this);
	this->TailAlter = false;
	return *(this->LazyEnd);*/
	return TVector_t<Ty>::Iterator_t(this->TailIndex, this);
}




