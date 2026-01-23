// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.tga;

static void swap_red_blue(std::vector<uint8_t> &pixels, uint32_t w, uint32_t h)
{
	auto size = w * h * 3;
	for(auto i = decltype(size) {0u}; i < size; i += 3) {
		auto tmp = pixels.at(i);
		pixels.at(i) = pixels.at(i + 2);
		pixels.at(i + 2) = tmp;
	}
}
static void flip_y(std::vector<uint8_t> &pixels, uint32_t w, uint32_t h)
{
	const size_t stride = w * 3u;
	std::vector<uint8_t> row(stride);
	auto *low = pixels.data();
	auto *high = &pixels.at((h - 1u) * stride);
	for(; low < high; low += stride, high -= stride) {
		memcpy(row.data(), low, stride);
		memcpy(low, high, stride);
		memcpy(high, row.data(), stride);
	}
}
void pragma::util::tga::write_tga(std::shared_ptr<fs::VFilePtrInternalReal> &f, uint32_t w, uint32_t h, const uint8_t *pixelData)
{
	std::vector<uint8_t> pixels {};
	pixels.resize(w * h * 3);
	memcpy(pixels.data(), pixelData, pixels.size() * sizeof(pixels.front()));

	auto outPixels = pixels;
	swap_red_blue(outPixels, w, h);
	flip_y(outPixels, w, h);
	f->Write<char>((char)(0)); // Length of image ID field
	f->Write<char>((char)(0)); // No color map included
	f->Write<char>(2);         // Uncompressed true-color image

	// Color map
	f->Write<short>((short)(0));
	f->Write<short>((short)(0));
	f->Write<char>((char)(0));

	f->Write<short>((short)(0)); // x-origin (origin = lower-left corner)
	f->Write<short>((short)(0)); // y-origin

	f->Write<char>(w % 256);
	f->Write<char>(CInt8(w / 256));

	f->Write<char>(h % 256);
	f->Write<char>(CInt8(h / 256));

	f->Write<char>(24);        // Bits per pixel
	f->Write<char>((char)(0)); // Image descriptor

	f->Write(outPixels.data(), outPixels.size());
}
void pragma::util::tga::write_tga(fs::VFilePtrReal &f, uint32_t w, uint32_t h, const std::vector<uint8_t> &pixels) { write_tga(f, w, h, pixels.data()); }
bool pragma::util::tga::write_tga(const std::string &fileName, uint32_t w, uint32_t h, const std::vector<uint8_t> &pixels)
{
	auto f = fs::open_file<fs::VFilePtrReal>(fileName, fs::FileMode::Write | fs::FileMode::Binary);
	if(f == nullptr)
		return false;
	write_tga(f, w, h, pixels);
	return true;
}
