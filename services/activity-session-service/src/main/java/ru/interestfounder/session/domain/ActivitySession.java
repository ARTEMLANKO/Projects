package ru.interestfounder.session.domain;

import jakarta.persistence.*;

import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "activity_sessions")
public class ActivitySession {

    @Id
    private UUID id;

    @Column(name = "user_id", nullable = false)
    private UUID userId;

    @Column(name = "district_id", nullable = false)
    private UUID districtId;

    @Column(name = "text", nullable = false, length = 1000)
    private String text;

    @Column(name = "starts_at", nullable = false)
    private Instant startsAt;

    @Column(name = "expires_at", nullable = false)
    public Instant expiresAt;

    @Enumerated(EnumType.STRING)
    @Column(name = "status", nullable = false, length = 20)
    private ActivitySessionStatus status;

    protected ActivitySession() {
    }

    public ActivitySession(UUID id,
                           UUID userId,
                           UUID districtId,
                           String text,
                           Instant startsAt,
                           Instant expiresAt,
                           ActivitySessionStatus status) {
        this.id = id;
        this.userId = userId;
        this.districtId = districtId;
        this.text = text;
        this.startsAt = startsAt;
        this.expiresAt = expiresAt;
        this.status = status;
    }

    public static ActivitySession create(UUID userId,
                                         UUID districtId,
                                         String text,
                                         Instant now) {
        return new ActivitySession(
                UUID.randomUUID(),
                userId,
                districtId,
                text,
                now,
                now.plusSeconds(3600),
                ActivitySessionStatus.ACTIVE
        );
    }

    public boolean isEffectivelyActive(Instant now) {
        return status == ActivitySessionStatus.ACTIVE && expiresAt.isAfter(now);
    }

    public void cancel() {
        if (status != ActivitySessionStatus.ACTIVE) {
            return;
        }
        this.status = ActivitySessionStatus.CANCELLED;
    }

    public UUID id() { return id; }
    public UUID userId() { return userId; }
    public UUID districtId() { return districtId; }
    public String text() { return text; }
    public Instant startsAt() { return startsAt; }
    public Instant expiresAt() { return expiresAt; }
    public ActivitySessionStatus status() { return status; }
}