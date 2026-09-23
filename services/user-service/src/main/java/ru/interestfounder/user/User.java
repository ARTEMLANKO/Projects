package ru.interestfounder.user;

import jakarta.persistence.*;

import java.util.UUID;

@Entity
@Table(name = "users")
public class User {

    @Id
    private UUID id;

    @Column(name = "telegram_user_id", unique = true)
    private long telegramUserId;

    @Column(length = 32)
    public String username;

    @ManyToOne
    @JoinColumn(name = "district_id")
    private District district;

    protected User() {
    }

    public static User create(long telegramUserId, String username) {
        var user = new User();
        user.id = UUID.randomUUID();
        user.telegramUserId = telegramUserId;
        user.username = username;
        return user;
    }


    public void selectDistrict(District district) {
        this.district = district;
    }

    public UUID getId() { return id; }
    public long getTelegramUserId() { return telegramUserId; }
    public String getUsername() { return username; }
    public District getDistrict() { return district; }

}

@Entity
@Table(name = "districts")
class District {

    @Id
    private UUID id;

    @Column(name = "name_ru", nullable = false, unique = true, length = 128)
    private String nameRu;

    protected District() {
    }

    public static District create(String nameRu) {
        var district = new District();
        district.id = UUID.randomUUID();
        district.nameRu = nameRu;
        return district;
    }

    public UUID getId() { return id; }
    public String getNameRu() { return nameRu; }
}