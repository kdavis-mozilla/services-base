/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include <time.h>
#include <stdio.h>

#ifdef _WIN32
/* Disable deprecation warnings */
#pragma warning(disable: 4996)
#endif

#include <alljoyn/notification/NotificationService.h>
#include <alljoyn/notification/NotificationText.h>
#include <alljoyn/notification/Notification.h>
#include "NotificationReceiverImpl.h"
#include <alljoyn/Status.h>
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace ajn;
using namespace services;
using namespace qcc;

static char const* const QCC_MODULE = "ServerSample";

NotificationReceiverImpl::NotificationReceiverImpl() {
}

NotificationReceiverImpl::~NotificationReceiverImpl() {
}

void NotificationReceiverImpl::Receive(Notification const& notification) {

    QCC_LogMsg(("Received message with msgId: %d; msgType: %d; from DeviceId: %s; DeviceName: %s; AppId: %s; AppName: %s; SenderBusName: %s;",
                notification.getMessageId(), notification.getMessageType(), notification.getDeviceId(),
                notification.getDeviceName(), notification.getAppId(), notification.getAppName(),
                notification.getSenderBusName()));

    // output the message text(s)
    std::vector<NotificationText> vecMessages = notification.getText();
    for (std::vector<NotificationText>::const_iterator vecMessage_it = vecMessages.begin(); vecMessage_it != vecMessages.end(); ++vecMessage_it) {
        QCC_LogMsg(("Message[%s]: %s;", vecMessage_it->getLanguage().c_str(),
                    vecMessage_it->getText().c_str()));
    }

}

void NotificationReceiverImpl::Dismiss(const int32_t msgId, const qcc::String appId)
{
    QCC_LogMsg(("Received dismiss for msgId=%d appId=%s", msgId, appId.c_str()));
}
