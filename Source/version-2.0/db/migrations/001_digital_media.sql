-- Migration: Add digital media support
-- Adds media_type for digital content and tracking table

-- Add DigitalMedia type if not exists
INSERT INTO media_type (name, description)
VALUES ('DigitalMedia', 'Digital content such as eBooks, PDFs, and video')
ON CONFLICT (name) DO NOTHING;

-- Digital media metadata table
CREATE TABLE IF NOT EXISTS digital_media (
    id BIGSERIAL PRIMARY KEY,
    media_id BIGINT NOT NULL REFERENCES media(id) ON DELETE CASCADE,
    mime_type TEXT NOT NULL,
    s3_key TEXT NOT NULL,
    file_size BIGINT NOT NULL DEFAULT 0,
    drm_protected BOOLEAN DEFAULT FALSE,
    current_version INT DEFAULT 1,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_digital_media_media_id ON digital_media(media_id);
CREATE INDEX IF NOT EXISTS idx_digital_media_s3_key ON digital_media(s3_key);

-- Version history for digital media
CREATE TABLE IF NOT EXISTS digital_media_version (
    id BIGSERIAL PRIMARY KEY,
    digital_media_id BIGINT NOT NULL REFERENCES digital_media(id) ON DELETE CASCADE,
    version_number INT NOT NULL,
    s3_key TEXT NOT NULL,
    file_size BIGINT NOT NULL DEFAULT 0,
    checksum TEXT,
    uploaded_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    UNIQUE (digital_media_id, version_number)
);

CREATE INDEX IF NOT EXISTS idx_dmv_digital_media_id ON digital_media_version(digital_media_id);
