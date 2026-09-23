package ru.interestfounder.session.infrastructure;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;
import ru.interestfounder.session.domain.ActivitySession;

import java.time.Instant;
import java.util.List;
import java.util.UUID;

@Repository
public interface JpaActivitySessionRepository extends JpaRepository<ActivitySession, UUID> {


    @Query("""
            SELECT s FROM ActivitySession s
            WHERE s.userId = :userId
              AND s.status = ru.interestfounder.session.domain.ActivitySessionStatus.ACTIVE
            ORDER BY s.startsAt DESC
            """)
    List<ActivitySession> findActiveByUserId(@Param("userId") UUID userId);

    @Query("""
            SELECT s FROM ActivitySession s
            WHERE s.text = :topicKey
              AND s.districtId = :districtId
              AND s.status = ru.interestfounder.session.domain.ActivitySessionStatus.ACTIVE
              AND s.userId <> :excludeUserId
              AND s.expiresAt > :now
            """)
    List<ActivitySession> findMatches(@Param("topicKey") String topicKey,
                                            @Param("districtId") UUID districtId,
                                            @Param("excludeUserId") UUID excludeUserId,
                                            @Param("now") Instant now);
}