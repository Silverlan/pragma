// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.shared:util.mvp_base;

export import pragma.math;

export {
	template<int i>
	class MVPBase {
	protected:
		Mat4 m_matModel = umat::identity();
		Mat4 m_matView[i];
		Mat4 m_matProjection = umat::identity();
		Mat4 m_matTransformation[i];

		void SetViewMatrix(const Mat4 &mat);
		void SetProjectionMatrix(const Mat4 &mat);
		void SetModelMatrix(const Mat4 &mat);
		void SetViewMatrix(const Mat4 &mat, unsigned int j);
		void UpdateTransformationMatrices();
		virtual void UpdateTransformationMatrix(unsigned int j);
	public:
		Mat4 &GetViewMatrix();
		Mat4 &GetProjectionMatrix();
		Mat4 &GetTransformationMatrix();
		Mat4 &GetModelMatrix();
		Mat4 &GetViewMatrix(unsigned int j);
		Mat4 &GetTransformationMatrix(unsigned int j);
	};

	template<int i>
	Mat4 &MVPBase<i>::GetViewMatrix()
	{
		return m_matView[0];
	}

	template<int i>
	Mat4 &MVPBase<i>::GetProjectionMatrix()
	{
		return m_matProjection;
	}

	template<int i>
	Mat4 &MVPBase<i>::GetModelMatrix()
	{
		return m_matModel;
	}

	template<int i>
	Mat4 &MVPBase<i>::GetTransformationMatrix()
	{
		return m_matTransformation[0];
	}

	template<int i>
	Mat4 &MVPBase<i>::GetViewMatrix(unsigned int j)
	{
		return m_matView[j];
	}

	template<int i>
	Mat4 &MVPBase<i>::GetTransformationMatrix(unsigned int j)
	{
		return m_matTransformation[j];
	}

	template<int i>
	void MVPBase<i>::SetViewMatrix(const Mat4 &mat)
	{
		m_matView[0] = mat;
		UpdateTransformationMatrix(0);
	}

	template<int i>
	void MVPBase<i>::SetProjectionMatrix(const Mat4 &mat)
	{
		m_matProjection = mat;
		UpdateTransformationMatrices();
	}

	template<int i>
	void MVPBase<i>::SetModelMatrix(const Mat4 &mat)
	{
		m_matModel = mat;
		UpdateTransformationMatrices();
	}

	template<int i>
	void MVPBase<i>::SetViewMatrix(const Mat4 &mat, unsigned int j)
	{
		m_matView[j] = mat;
		UpdateTransformationMatrix(j);
	}

	template<int i>
	void MVPBase<i>::UpdateTransformationMatrices()
	{
		for(unsigned int j = 0; j < i; j++)
			UpdateTransformationMatrix(j);
	}

	template<int i>
	void MVPBase<i>::UpdateTransformationMatrix(unsigned int j)
	{
		m_matTransformation[j] = m_matProjection * m_matView[j] * m_matModel;
	}

	////////////////////////////////

	template<int i>
	class MVPBias : public MVPBase<i> {
	protected:
		Mat4 m_matBiasTransformation[i];
		virtual void UpdateTransformationMatrix(unsigned int j) override;
	public:
		Mat4 &GetBiasTransformationMatrix();
		Mat4 &GetBiasTransformationMatrix(unsigned int j);
	};

	template<int i>
	Mat4 &MVPBias<i>::GetBiasTransformationMatrix(unsigned int j)
	{
		return m_matBiasTransformation[j];
	}

	template<int i>
	Mat4 &MVPBias<i>::GetBiasTransformationMatrix()
	{
		return m_matBiasTransformation[0];
	}

	template<int i>
	void MVPBias<i>::UpdateTransformationMatrix(unsigned int j)
	{
		MVPBase<i>::UpdateTransformationMatrix(j);
		m_matBiasTransformation[j] = umat::BIAS * MVPBase<i>::m_matTransformation[j];
	}
};
