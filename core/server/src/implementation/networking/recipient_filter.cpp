// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :networking.recipient_filter;

import :networking.iserver_client;

pragma::networking::ClientRecipientFilter::ClientRecipientFilter(const std::function<bool(const IServerClient &)> &filter) : m_filter {filter} {}
pragma::networking::ClientRecipientFilter::ClientRecipientFilter(const IServerClient &client, FilterType filterType)
{
	switch(filterType) {
	case FilterType::Include:
		m_filter = [&client](const IServerClient &clOther) -> bool { return &clOther == &client; };
		break;
	case FilterType::Exclude:
	default:
		m_filter = [&client](const IServerClient &clOther) -> bool { return &clOther != &client; };
		break;
	}
}
pragma::networking::ClientRecipientFilter::ClientRecipientFilter() : ClientRecipientFilter {[](const IServerClient &) -> bool { return true; }} {}
bool pragma::networking::ClientRecipientFilter::operator()(const IServerClient &cl) const { return m_filter(cl); }

/////////////////

const std::vector<pragma::util::WeakHandle<const pragma::networking::IServerClient>> &pragma::networking::TargetRecipientFilter::GetRecipients() const { return m_recipients; }
void pragma::networking::TargetRecipientFilter::AddRecipient(const IServerClient &client) { m_recipients.push_back(client.shared_from_this()); }
void pragma::networking::TargetRecipientFilter::RemoveRecipient(const IServerClient &client)
{
	auto it = std::find_if(m_recipients.begin(), m_recipients.end(), [&client](const util::WeakHandle<const IServerClient> &hClientOther) { return hClientOther.get() == &client; });
	if(it == m_recipients.end())
		return;
	m_recipients.erase(it);
}
bool pragma::networking::TargetRecipientFilter::HasRecipient(const IServerClient &client) const
{
	return std::find_if(m_recipients.begin(), m_recipients.end(), [&client](const util::WeakHandle<const IServerClient> &hClientOther) { return hClientOther.get() == &client; }) != m_recipients.end();
}

void pragma::networking::TargetRecipientFilter::SetFilterType(ClientRecipientFilter::FilterType filterType) { m_filterType = filterType; }
pragma::networking::ClientRecipientFilter::FilterType pragma::networking::TargetRecipientFilter::GetFilterType() const { return m_filterType; }

pragma::networking::TargetRecipientFilter::operator pragma::networking::ClientRecipientFilter() const
{
	auto recipients = m_recipients;
	auto filterType = m_filterType;
	return ClientRecipientFilter {[recipients, filterType](const IServerClient &client) -> bool {
		auto result = std::find_if(recipients.begin(), recipients.end(), [&client](const util::WeakHandle<const IServerClient> &hClientOther) { return &client == hClientOther.get(); }) != recipients.end();
		if(filterType == ClientRecipientFilter::FilterType::Include)
			return result;
		return !result;
	}};
}
