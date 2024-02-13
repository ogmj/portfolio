
template < typename T >
class Singleton
{
public:
	Singleton()
	{
	}
	static T & GetStaticInstance()
	{
		static T ms_Instance;
		return ms_Instance;
	}

	static T * GetInstance()
	{
		if( ms_pInstance == NULL )
		{
			ms_pInstance = new T;
			atexit( ReleaseInstance );
		}
		return ms_pInstance;
	}
	static void ReleaseInstance()
	{
		if (ms_pInstance)
		{
			delete ms_pInstance;
		}
	}

private:
	static T * ms_pInstance;
	static T ms_Instance;
};

template< typename T >
T *	Singleton< T >::ms_pInstance = nullptr; 

