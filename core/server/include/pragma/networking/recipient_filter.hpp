/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_RECIPIENT_FILTER_HPP__
#define __PRAGMA_RECIPIENT_FILTER_HPP__

#include "pragma/serverdefinitions.h"
#include <functional>

namespace pragma::networking
{
	class IServerClient;
	class DLLSERVER ClientRecipientFilter
	{
	public:
		enum class FilterType : uint8_t
		{
			Include = 0,
			Exclude
		};

		ClientRecipientFilter(const std::function<bool(const IServerClient&)> &filter);
		ClientRecipientFilter(const IServerClient &client,FilterType filterType=FilterType::Include);
		ClientRecipientFilter();
		bool operator()(const IServerClient&) const;
	private:
		std::function<bool(const IServerClient&)> m_filter = nullptr;
	};

	class DLLSERVER TargetRecipientFilter
	{
	public:
		TargetRecipientFilter()=default;
		const std::vector<util::WeakHandle<const IServerClient>> &GetRecipients() const;
		void AddRecipient(const IServerClient &client);
		void RemoveRecipient(const IServerClient &client);
		bool HasRecipient(const IServerClient &client) const;

		void SetFilterType(ClientRecipientFilter::FilterType filterType);
		ClientRecipientFilter::FilterType GetFilterType() const;

		operator ClientRecipientFilter() const;
	private:
		std::vector<util::WeakHandle<const IServerClient>> m_recipients = {};
		ClientRecipientFilter::FilterType m_filterType = ClientRecipientFilter::FilterType::Include;
	};
};

#endif
