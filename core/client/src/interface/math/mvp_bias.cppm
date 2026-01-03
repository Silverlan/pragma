// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:math.mvp_bias;

export import pragma.math;

export namespace pragma::math {
	template<int i>
	class MVPBase {
	  protected:
		Mat4 m_matModel = umat::identity();
		Mat4 m_matView[i];
		Mat4 m_matProjection = umat::identity();
		Mat4 m_matTransformation[i];

		void SetViewMatrix(const Mat4 &mat)
		{
			m_matView[0] = mat;
			UpdateTransformationMatrix(0);
		}
		void SetProjectionMatrix(const Mat4 &mat)
		{
			m_matProjection = mat;
			UpdateTransformationMatrices();
		}
		void SetModelMatrix(const Mat4 &mat)
		{
			m_matModel = mat;
			UpdateTransformationMatrices();
		}
		void SetViewMatrix(const Mat4 &mat, unsigned int j)
		{
			m_matView[j] = mat;
			UpdateTransformationMatrix(j);
		}
		void UpdateTransformationMatrices()
		{
			for(unsigned int j = 0; j < i; j++)
				UpdateTransformationMatrix(j);
		}
		virtual void UpdateTransformationMatrix(unsigned int j) { m_matTransformation[j] = m_matProjection * m_matView[j] * m_matModel; }
	  public:
		Mat4 &GetViewMatrix() { return m_matView[0]; }
		Mat4 &GetProjectionMatrix() { return m_matProjection; }
		Mat4 &GetTransformationMatrix() { return m_matTransformation[0]; }
		Mat4 &GetModelMatrix() { return m_matModel; }
		Mat4 &GetViewMatrix(unsigned int j) { return m_matView[j]; }
		Mat4 &GetTransformationMatrix(unsigned int j) { return m_matTransformation[j]; }
	};

	////////////////////////////////

	template<int i>
	class MVPBias : public MVPBase<i> {
	  protected:
		Mat4 m_matBiasTransformation[i];
		virtual void UpdateTransformationMatrix(unsigned int j) override
		{
			MVPBase<i>::UpdateTransformationMatrix(j);
			m_matBiasTransformation[j] = umat::PRM_BIAS * MVPBase<i>::m_matTransformation[j];
		}
	  public:
		Mat4 &GetBiasTransformationMatrix() { return m_matBiasTransformation[0]; }
		Mat4 &GetBiasTransformationMatrix(unsigned int j) { return m_matBiasTransformation[j]; }
	};
}
