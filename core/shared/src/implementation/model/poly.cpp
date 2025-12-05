// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.poly;

Con::c_cout &operator<<(Con::c_cout &os, const Poly &poly)
{
	os << "Poly[" << &poly << "] [" << poly.m_vertices.size() << "] [" << poly.m__normal.x << " " << poly.m__normal.y << " " << poly.m__normal.z << "] [" << poly.m_distance << "]";
	return os;
}

static const double EPSILON = 4.9406564584125e-2; //4.9406564584125e-4;//4.94065645841247e-324;
Poly::Poly(pragma::NetworkState *nw) : m_nwState {nw} {}

TextureData *Poly::GetTextureData() { return m_texData.get(); }

void Poly::SetTextureData(std::string texture, Vector3 nu, Vector3 nv, float ou, float ov, float su, float sv, float rot)
{
	m_texData = std::make_unique<TextureData>();
	m_texData->texture = texture;
	m_texData->nu = nu;
	m_texData->nv = nv;
	m_texData->ou = ou;
	m_texData->ov = ov;
	m_texData->su = su;
	m_texData->sv = sv;
	m_texData->rot = rot;
}
PolyInfo &Poly::GetCompiledData() { return m_compiledData; }
msys::Material *Poly::GetMaterial() { return m_material; }
void Poly::SetMaterial(msys::Material *mat) { m_material = mat; }

void Poly::SetDistance(double d) { m_distance = d; }
void Poly::SetNormal(Vector3 n)
{
	m__normal.x = n.x;
	m__normal.y = n.y;
	m__normal.z = n.z;
}

Vector3 Poly::GetCalculatedNormal() { return m_normal; }

const short Poly::ClassifyPoint(Vector3 *point)
{
	Vector3 pos = GetCalculatedNormal() * float(-GetDistance());
	Vector3 v = (pos - (*point));
	uvec::normalize(&v);
	float r = glm::dot(v, GetNormal());
	if(r > EPSILON)
		return CLASSIFY_FRONT;
	else if(r < EPSILON)
		return CLASSIFY_BACK;
	return CLASSIFY_ONPLANE;
}

bool Poly::IsValid() { return GetVertexCount() >= 3; }

double Poly::GetDistance() { return m_distance; }
Vector3 Poly::GetNormal() { return m__normal; }

bool Poly::HasVertex(Vector3 *vert)
{
	for(int i = 0; i < m_vertices.size(); i++) {
		auto &vertex = m_vertices[i];
		Vector3 v = vertex.pos;
		Vector3 sub(v);
		uvec::sub(&sub, *vert);
		if(sub.x <= EPSILON && sub.x >= -EPSILON && sub.y <= EPSILON && sub.y >= -EPSILON && sub.z <= EPSILON && sub.z >= -EPSILON)
			return true;
	}
	return false;
}

bool Poly::AddUniqueVertex(Vector3 vert, Vector3 n)
{
	if(HasVertex(&vert))
		return false;
	AddVertex(vert, n);
	return true;
}

void Poly::AddVertex(Vector3 vert, Vector3 n) { m_vertices.push_back(PolyVertex(vert, n)); }

unsigned int Poly::GetVertexCount() { return static_cast<unsigned int>(m_vertices.size()); }

void Poly::debug_print()
{
	Con::cout << "Poly '" << *this << "':" << Con::endl;
	for(int i = 0; i < m_vertices.size(); i++)
		Con::cout << "\tVertex (" << m_vertices[i].pos.x << "," << m_vertices[i].pos.y << "," << m_vertices[i].pos.z << ")" << Con::endl;
}

void Poly::SortVertices()
{
	if(!IsValid())
		return;
	Vector3 a(m_vertices[1].pos);
	uvec::sub(&a, m_vertices[0].pos);
	Vector3 an(a);
	Vector3 bn = {};
	a = glm::normalize(a);
	auto numVerts = GetVertexCount();
	double BiggestAngle;
	unsigned int tempvert1 = 0;
	unsigned int tempvert2;
	unsigned int biggestVert = 0;
	for(size_t i = 0; i < (numVerts > 0 ? (numVerts - 1) : 0); i++) {
		BiggestAngle = 1;
		tempvert2 = tempvert1 + 1;
		for(size_t j = i + 1; j < numVerts; j++) {
			Vector3 b(m_vertices[tempvert2].pos);
			uvec::sub(&b, m_vertices[tempvert1].pos);
			bn = Vector3(b);
			b = glm::normalize(b);

			float dotProd = glm::dot(a, b);
			if(dotProd < BiggestAngle) {
				BiggestAngle = dotProd;
				biggestVert = tempvert2;
			}
			tempvert2++;
		}
		if(biggestVert == 0)
			break;
		a = Vector3(m_vertices[tempvert1].pos);
		uvec::sub(&a, m_vertices[biggestVert].pos);
		an = Vector3(a);
		a = glm::normalize(a);

		if(biggestVert != tempvert1 + 1) {
			Vector3 v(m_vertices[tempvert1 + 1].pos);
			m_vertices[tempvert1 + 1].pos = m_vertices[biggestVert].pos;
			m_vertices[biggestVert].pos = v;
		}
		tempvert1++;
	}
	Vector3 n(m__normal);
	CalculatePlane();
	CalculateBounds();
	if(glm::dot(m_normal, n) > 0)
		ReverseVertexOrder();
}

bool Poly::CalculatePlane()
{
	if(!IsValid())
		return false;
	Vector3 centerOfMass(0, 0, 0);
	Vector3 n(0, 0, 0);
	size_t j = 0;
	auto numVerts = m_vertices.size();
	for(size_t i = 0; i < numVerts; i++) {
		j = i + 1;
		if(j >= numVerts)
			j = 0;
		n.x += (m_vertices[i].pos.y - m_vertices[j].pos.y) * (m_vertices[i].pos.z + m_vertices[j].pos.z);
		n.y += (m_vertices[i].pos.z - m_vertices[j].pos.z) * (m_vertices[i].pos.x + m_vertices[j].pos.x);
		n.z += (m_vertices[i].pos.x - m_vertices[j].pos.x) * (m_vertices[i].pos.y + m_vertices[j].pos.y);

		uvec::add(&centerOfMass, m_vertices[i].pos);
	}
	float magnitude = glm::length(n);
	n /= magnitude;
	centerOfMass /= numVerts;
	m_centerOfMass = centerOfMass;
	m_normal = n;
	Vector3 c = -centerOfMass;
	m_distance = glm::dot(c, n);
	return true;
}

Vector3 *Poly::GetWorldPosition() { return &m_centerOfMass; }

void Poly::CalculateBounds()
{
	Vector3 center = GetCenter();
	Vector3 min = {};
	Vector3 max = {};
	uvec::max(&min);
	uvec::min(&max);
	for(int i = 0; i < m_vertices.size(); i++) {
		Vector3 pos = m_vertices[i].pos;
		Vector3 diff(pos);
		uvec::sub(&diff, center);
		uvec::min(&min, diff);
		uvec::max(&max, diff);
	}
	//Con::cout<<"Poly Bounds: "<<Vector3::tostring(&min)<<" - "<<Vector3::tostring(&max)<<Con::endl;
	m_min = min;
	m_max = max;
}

Vector3 Poly::GetCenter() { return m_center; }

void Poly::ReverseVertexOrder()
{
	auto numVerts = m_vertices.size();
	unsigned int h = static_cast<unsigned int>(static_cast<float>(numVerts) * 0.5f);
	for(unsigned int i = 0; i < h; i++) {
		Vector3 pos = m_vertices[i].pos;
		m_vertices[i].pos = m_vertices[numVerts - i - 1].pos;
		m_vertices[numVerts - i - 1].pos = pos;
	}
}

std::vector<PolyVertex> &Poly::GetVertices() { return m_vertices; }

void Poly::GetBounds(Vector3 *min, Vector3 *max)
{
	min->x = m_min.x;
	min->y = m_min.y;
	min->z = m_min.z;
	max->x = m_max.x;
	max->y = m_max.y;
	max->z = m_max.z;
}

void Poly::Localize(const Vector3 &center)
{
	if(m_centerLocalized.has_value() == false)
		*m_centerLocalized = Vector3(center);
	else
		uvec::add(&*m_centerLocalized, center);
	for(int i = 0; i < m_vertices.size(); i++)
		uvec::sub(&m_vertices[i].pos, center);
	Calculate();
}

void Poly::Calculate()
{
	CalculateNormal();
	CalculatePlane();
	CalculateBounds();
	// TODO: CalculateTextureAxes()
}

void Poly::CalculateNormal()
{
	auto numVerts = m_vertices.size();
	if(numVerts < 2)
		return;
	Vector3 a = m_vertices[0].pos - m_vertices[numVerts - 1].pos;
	Vector3 b = m_vertices[1].pos - m_vertices[0].pos;
	Vector3 n = glm::normalize(glm::cross(a, b));
	Vector3 nd = -n;
	float d = glm::dot(nd, m_vertices[numVerts - 1].pos);
	SetNormal(n);
	SetDistance(d);
}

bool Poly::GenerateTriangleMesh(std::vector<uint16_t> *triangles, std::vector<glm::vec2> *uvs, std::vector<glm::vec3> *normals)
{
	if(!IsValid())
		return false;
	unsigned int pivot = 0;
	auto &a = m_vertices[pivot];
	unsigned int numVerts = static_cast<unsigned int>(m_vertices.size());
	triangles->reserve(numVerts * 3u);
	for(unsigned int i = pivot + 2; i < numVerts; i++) {
		auto &b = m_vertices[i - 1];
		auto &c = m_vertices[i];
		triangles->push_back(pivot);
		triangles->push_back(i - 1);
		triangles->push_back(i);

		uvs->push_back(glm::vec2(a.u, a.v));
		uvs->push_back(glm::vec2(b.u, b.v));
		uvs->push_back(glm::vec2(c.u, c.v));

		normals->push_back(a.normal);
		normals->push_back(b.normal);
		normals->push_back(c.normal);
	}
	return true;
}

void Poly::RemoveDisplacement()
{
	if(m_displacement == nullptr)
		return;
	m_displacement = nullptr;
	m_material = m_nwState->GetMaterialManager().LoadAsset("tools/toolsnodraw").get();
}

void Poly::SetDisplacement(std::shared_ptr<DispInfo> disp) { m_displacement = disp; }

std::shared_ptr<DispInfo> Poly::GetDisplacement() { return m_displacement; }
bool Poly::IsDisplacement() { return (m_displacement == nullptr) ? false : true; }

static void build_displacement_triangles(std::vector<glm::vec3> &sideVerts, unsigned int start, glm::vec3 &nu, glm::vec3 &nv, float sw, float sh, float ou, float ov, float su, float sv, unsigned char power, std::vector<std::vector<glm::vec3>> &normals,
  std::vector<std::vector<glm::vec3>> &offsets, std::vector<std::vector<float>> &distances, unsigned char numAlpha, std::vector<std::vector<glm::vec2>> &alphas, std::vector<glm::vec3> &outVertices, std::vector<glm::vec2> &outUvs, std::vector<unsigned int> &outTriangles,
  std::vector<glm::vec3> &faceNormals, std::vector<glm::vec2> *outAlphas)
{
	int rows = umath::pow(2, CInt32(power)) + 1;
	unsigned int numVerts = rows * rows;
	outUvs.resize(numVerts);
	if(numAlpha > 0)
		outAlphas->resize(numVerts);
	glm::vec3 sortedSideVerts[4];
	char j = 0;
	for(auto i = start; i < CUInt32(sideVerts.size()); i++) {
		sortedSideVerts[j] = sideVerts[i];
		j++;
	}
	for(unsigned int i = 0; i < start; i++) {
		sortedSideVerts[j] = sideVerts[i];
		j++;
	}
	glm::vec3 &x1 = sortedSideVerts[0];
	glm::vec3 &x2 = sortedSideVerts[1];
	glm::vec3 &y1 = sortedSideVerts[3];
	glm::vec3 &y2 = sortedSideVerts[2];
	glm::vec3 xOffset1 = (x2 - x1) / float(rows - 1);
	glm::vec3 xOffset2 = (y2 - y1) / float(rows - 1);
	glm::vec3 yOffset = (y1 - x1) / float(rows - 1); // CHECKME
	outVertices.resize(numVerts);
	glm::vec3 cur = sortedSideVerts[0];
	for(int col = 0; col < rows; col++) {
		std::vector<glm::vec3> &cNormals = normals[col];
		std::vector<glm::vec2> *cAlphas = (numAlpha > 0) ? &alphas[col] : nullptr;
		std::vector<float> &cDistances = distances[col];
		std::vector<glm::vec3> &cOffsets = offsets[col];

		glm::vec3 rowPos = cur;
		float offsetScale = col / float(rows - 1);
		for(int row = 0; row < rows; row++) {
			unsigned int idx = col * rows + row;
			//outNormals[idx] = -cNormals[row]; // This is the offset normal, not the actual face normal!
			if(numAlpha > 0)
				(*outAlphas)[idx] = (*cAlphas)[row];

			glm::vec3 vA = rowPos + cNormals[row] * cDistances[row] + cOffsets[row];
			glm::vec2 uv;
			uv.x = (glm::dot(rowPos, nu) * sw) / su + ou * sw;
			uv.y = 1.f - ((glm::dot(rowPos, nv) * sh) / sv + ov * sh);
			outUvs[idx] = uv;
			outVertices[idx] = vA;

			glm::vec3 xOffset = (1.f - offsetScale) * xOffset1 + offsetScale * xOffset2;
			rowPos += xOffset;
		}
		cur = sortedSideVerts[0] + (yOffset * float(col + 1));
		//if(col < rows -1)
		//	cur += yOffset +normals[col +1][0] *distances[col +1][0]; // +cNormals[0] *cDistances[0]; TODO!! -> Next column!
	}
	outTriangles.resize((rows - 1) * (rows - 1) * 6);
	unsigned int idx = 0;
	for(int col = 0; col < rows - 1; col++) {
		for(int row = 0; row < rows - 1; row++) {
			int a = col * rows + row;
			int b = a + 1;
			int c = (col + 1) * rows + row;
			outTriangles[idx] = a;
			outTriangles[idx + 1] = b;
			outTriangles[idx + 2] = c;

			// Calculate Face Normals
			auto na = -uvec::cross(outVertices[c] - outVertices[a], outVertices[b] - outVertices[a]);
			uvec::normalize(&na);
			faceNormals.push_back(na);
			//

			idx += 3;
			int d = (col + 1) * rows + row + 1;
			outTriangles[idx] = b;
			outTriangles[idx + 1] = d;
			outTriangles[idx + 2] = c;

			// Calculate Face Normals
			auto nb = -uvec::cross(outVertices[c] - outVertices[b], outVertices[d] - outVertices[b]);
			uvec::normalize(&nb);
			faceNormals.push_back(nb);
			//

			idx += 3;
		}
	}
}

void Poly::BuildDisplacement(std::vector<glm::vec3> &verts, glm::vec3 &nu, glm::vec3 &nv, unsigned int w, unsigned int h, std::vector<glm::vec3> &outVertices, std::vector<glm::vec2> &outUvs, std::vector<unsigned int> &outTriangles, std::vector<glm::vec3> &faceNormals,
  unsigned char &numAlpha, std::vector<glm::vec2> *outAlphas)
{
	// Texture coordinates
	auto texData = GetTextureData();
	float sw = 1.f / w;
	float sh = 1.f / h;
	float ou = texData->ou;
	float ov = texData->ov;
	float su = texData->su;
	float sv = texData->sv;
	//

	// Vertex Ids
	auto disp = GetDisplacement();
	auto numVerts = static_cast<unsigned int>(verts.size());
	auto &vertices = GetVertices();
	auto numPolyVerts = static_cast<unsigned int>(vertices.size());
	std::vector<unsigned int> vertIds;
	for(unsigned int k = 0; k < numPolyVerts; k++) {
		auto &va = vertices[k].pos;
		for(unsigned int l = 0; l < numVerts; l++) {
			auto &vb = verts[l];
			if(fabsf(va.x - vb.x) <= EPSILON && fabsf(va.y - vb.y) <= EPSILON && fabsf(va.z - vb.z) <= EPSILON) {
				vertIds.push_back(l);
				break;
			}
		}
	}
	//

	auto rows = static_cast<unsigned int>(pow(2, disp->power)) + 1;
	// Alpha
	bool bHasAlphaA = false;
	bool bHasAlphaB = false;
	for(int i = (rows - 1); i >= 0; i--) {
		std::vector<glm::vec2> &vAlphas = disp->alphas[i];
		for(unsigned int j = 0; j < rows; j++) {
			if(vAlphas[j].x != 0.f) {
				bHasAlphaA = true;
				if(bHasAlphaB == true)
					break;
			}
			if(vAlphas[j].y != 0.f) {
				bHasAlphaB = true;
				if(bHasAlphaA == true)
					break;
			}
		}
		if(bHasAlphaA == true && bHasAlphaB == true)
			break;
	}
	if(bHasAlphaB == true && bHasAlphaA == false)
		bHasAlphaB = false;
	numAlpha = 0;
	if(bHasAlphaB == true)
		numAlpha = 2;
	else if(bHasAlphaA == true)
		numAlpha = 1;
	//
	build_displacement_triangles(verts, disp->startpositionId, nu, nv, sw, sh, ou, ov, su, sv, disp->power, disp->normals, disp->offsets, disp->distances, numAlpha, disp->alphas, outVertices, outUvs, outTriangles, faceNormals, outAlphas);
}

PolyDispInfo::PolyDispInfo() : numAlpha(0), power(0) {}

PolyInfo::PolyInfo() : width(0.f), height(0.f) {}
