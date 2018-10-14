#program once
template<typename T>
class CInstanceCreator
{
public:
	static T* getInstance()
	{
		static T t;
		return &t;
	}
};
