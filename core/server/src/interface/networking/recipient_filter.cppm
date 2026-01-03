// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:networking.recipient_filter;

export import pragma.util;

export namespace pragma::networking {
	class IServerClient;
	class DLLSERVER ClientRecipientFilter {
	  public:
		enum class FilterType : uint8_t { Include = 0, Exclude };

		ClientRecipientFilter(const std::function<bool(const IServerClient &)> &filter);
		ClientRecipientFilter(const IServerClient &client, FilterType filterType = FilterType::Include);
		ClientRecipientFilter();
		bool operator()(const IServerClient &) const;
	  private:
		std::function<bool(const IServerClient &)> m_filter = nullptr;
	};

	class DLLSERVER TargetRecipientFilter {
	  public:
		TargetRecipientFilter() = default;
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
