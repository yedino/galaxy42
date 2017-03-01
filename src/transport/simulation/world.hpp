
#pragma once


class c_world {
	public:
		c_world()=default;
		virtual ~c_world()=default;

		int generate_simul_transport();

	protected:
		int m_simul_transport=0;
};




