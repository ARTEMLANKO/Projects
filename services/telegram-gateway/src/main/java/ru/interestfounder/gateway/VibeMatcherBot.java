package ru.interestfounder.gateway;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;
import org.telegram.telegrambots.client.okhttp.OkHttpTelegramClient;
import org.telegram.telegrambots.longpolling.interfaces.LongPollingUpdateConsumer;
import org.telegram.telegrambots.longpolling.starter.SpringLongPollingBot;
import org.telegram.telegrambots.meta.api.methods.AnswerCallbackQuery;
import org.telegram.telegrambots.meta.api.methods.send.SendMessage;
import org.telegram.telegrambots.meta.api.methods.updatingmessages.EditMessageText;
import org.telegram.telegrambots.meta.api.objects.CallbackQuery;
import org.telegram.telegrambots.meta.api.objects.Update;
import org.telegram.telegrambots.meta.api.objects.message.Message;
import org.telegram.telegrambots.meta.api.objects.replykeyboard.InlineKeyboardMarkup;
import org.telegram.telegrambots.meta.api.objects.replykeyboard.buttons.InlineKeyboardButton;
import org.telegram.telegrambots.meta.api.objects.replykeyboard.buttons.InlineKeyboardRow;
import org.telegram.telegrambots.meta.exceptions.TelegramApiException;
import org.telegram.telegrambots.meta.generics.TelegramClient;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

@Component
public class VibeMatcherBot implements SpringLongPollingBot {


    private final String token;
    private final UserServiceClient userService;
    private final ActivityServiceClient activityService;
    private final TelegramClient client;

    public VibeMatcherBot(
            @Value("${telegram.bot.token}") String token,
            UserServiceClient userService,
            ActivityServiceClient activityService
    ) {
        this.token = token;
        this.userService = userService;
        this.activityService = activityService;
        this.client = new OkHttpTelegramClient(token);
    }

    @Override
    public String getBotToken() {
        return token;
    }

    @Override
    public LongPollingUpdateConsumer getUpdatesConsumer() {
        return this::handle;
    }

    private void handle(List<Update> updates) {
        for (var update : updates) {
            if (update.hasMessage()) {
                onMessage(update.getMessage());
            } else if (update.hasCallbackQuery()) {
                onCallback(update.getCallbackQuery());
            }
        }
    }

    private void onMessage(Message message) {
        String text = message.getText() == null ? "" : message.getText();
        switch (text.split(" ")[0]) {
            case "/start" -> start(message);
            case "/district" -> showDistricts(message);
            case "/activity" -> createActivity(message, text.split(" ")[1]);
            case "/matches" -> showMatches(message);
            case "/active" -> showActive(message);
            case "/cancel" -> cancelActivity(message);
            default -> send(message.getChatId(),
                    "Доступно: /start, /district, /activity, /matches, /active, /cancel");
        }
    }

    private void start(Message message) {
        var user = message.getFrom();
        var profile = userService.upsert(user.getId(), user.getUserName());

        if (profile.district() == null) {
            showDistricts(message);
        } else {
            String name = "@" + profile.username();
            send(message.getChatId(),
                    "С возвращением, " + name + "! Ваш район: " + profile.district().name());
        }
    }

    private void showDistricts(Message message) {
        var user = message.getFrom();
        userService.upsert(user.getId(), user.getUserName());

        var districts = userService.districts();

        var rows = new ArrayList<InlineKeyboardRow>();
        for (int i = 0; i < districts.size(); i += 2) {
            var row = new InlineKeyboardRow();
            row.add(button(districts.get(i)));
            if (i + 1 < districts.size()) {
                row.add(button(districts.get(i + 1)));
            }
            rows.add(row);
        }

        var keyboard = new InlineKeyboardMarkup(rows);
        execute(SendMessage.builder()
                .chatId(message.getChatId())
                .text("Выберите район Санкт-Петербурга:")
                .replyMarkup(keyboard)
                .build());
    }

    private InlineKeyboardButton button(UserServiceClient.District d) {
        return InlineKeyboardButton.builder()
                .text(d.name())
                .callbackData(d.id().toString())
                .build();
    }

    private void onCallback(CallbackQuery callback) {
        var user = callback.getFrom();
        String data = callback.getData();

        UUID districtId;
        districtId = UUID.fromString(data);
        userService.selectDistrict(user.getId(), districtId);
        answerCallback(callback.getId(), "Район сохранён");
    }

    private void createActivity(Message message, String args) {
        var user = message.getFrom();
        var profile = userService.upsert(user.getId(), user.getUserName());

        if (profile.district() == null) {
            send(message.getChatId(), "Сначала выберите район через /district");
            return;
        }
        activityService.create(profile.id(), profile.district().id(), args);
        send(message.getChatId(), "Активность создана: '" + args + "'");
    }

    private void showMatches(Message message) {
        var user = message.getFrom();
        var profile = userService.upsert(user.getId(), user.getUserName());

        List<ActivityServiceClient.Session> matches;
        try {
            matches = activityService.findMatches(profile.id());
        } catch (Exception e) {
            send(message.getChatId(), "Не удалось найти. Попробуйте позже.");
            return;
        }

        var sb = new StringBuilder("Найдено ").append(matches.size()).append(":\n");
        for (var s : matches) {
            String uname = userService.findByUUID(s.userId())
                    .map(UserServiceClient.Profile::username)
                    .map(u -> "@" + u)
                    .orElse(null);
            sb.append("• ").append(uname).append(" - ").append(s.text()).append("\n");
        }
        send(message.getChatId(), sb.toString());
    }

    private void showActive(Message message) {
        var user = message.getFrom();
        var profile = userService.upsert(user.getId(), user.getUserName());

        var active = activityService.findActive(profile.id());
        if (active.isEmpty()) {
            send(message.getChatId(), "Нет активной сессии. Создайте через /activity");
            return;
        }
        send(message.getChatId(), "Ваша сессия: '" + active.get().text() + "' до " + active.get().expiresAt());
    }

    private void cancelActivity(Message message) {
        var user = message.getFrom();
        var profile = userService.upsert(user.getId(), user.getUserName());

        var active = activityService.findActive(profile.id());
        if (active.isEmpty()) {
            send(message.getChatId(), "Нечего отменять.");
            return;
        }
        activityService.cancel(active.get().id());
        send(message.getChatId(), "Сессия отменена.");
    }

    private void send(long chatId, String text) {
        execute(SendMessage.builder().chatId(chatId).text(text).build());
    }

    private void answerCallback(String callbackId, String text) {
        execute(AnswerCallbackQuery.builder().callbackQueryId(callbackId).text(text).build());
    }

    private void execute(Object method) {
        try {
            if (method instanceof SendMessage m) {
                client.execute(m);
            } else if (method instanceof EditMessageText m) {
                client.execute(m);
            } else if (method instanceof AnswerCallbackQuery m) {
                client.execute(m);
            }
        } catch (TelegramApiException e) {}
    }
}