/* Copyright (c) 2013 The Squash Authors
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *   Evan Nemerson <evan@coeus-group.com>
 */

#include <assert.h>

#include "squash.h"
#include "internal.h"

/**
 * @var _SquashStream::base_object
 * @brief Base object.
 */

/**
 * @var _SquashStream::priv
 * @brief Private data.
 *
 * This is managed internally by Squash and should not be modified by
 * consumers or plugins.
 */

/**
 * @var _SquashStream::next_in
 * @brief The next input data to consume.
 */

/**
 * @var _SquashStream::avail_in
 * @brief Size (in bytes) of available input.
 */

/**
 * @var _SquashStream::total_in
 * @brief The total number of bytes input.
 *
 * This is managed internally by Squash and should not be modified by
 * consumers or plugins.
 */

/**
 * @var _SquashStream::next_out
 * @brief The buffer to write output to.
 */

/**
 * @var _SquashStream::avail_out
 * @brief Number of bytes available in the output buffer.
 */

/**
 * @var _SquashStream::total_out
 * @brief Total number of bytes output.
 *
 * This is managed internally by Squash and should not be modified by
 * consumers or plugins.
 */

/**
 * @var _SquashStream::codec
 * @brief Codec used for this stream.
 */

/**
 * @var _SquashStream::options
 * @brief Options used for this stream.
 */

/**
 * @var _SquashStream::stream_type
 * @brief Stream type.
 */

/**
 * @var _SquashStream::state
 * @brief State the stream is in.
 *
 * This is managed internally by Squash and should not be modified by
 * consumers or plugins.
 */

/**
 * @var _SquashStream::user_data
 * @brief User data
 *
 * Note that this is for consumers of the library, *not* for plugins.
 * It should be safe to use this from your application.
 */

/**
 * @var _SquashStream::destroy_user_data
 * @brief Squashlback to invoke on *user_data* when it is no longer
 *   necessary.
 */

/**
 * @defgroup SquashStream SquashStream
 * @brief Low-level compression and decompression streams.
 *
 * @{
 */

/**
 * @enum _SquashStreamType
 * @brief Stream type.
 */

/**
 * @var _SquashStreamType::SQUASH_STREAM_COMPRESS
 * @brief A compression stream.
 */

/**
 * @var _SquashStreamType::SQUASH_STREAM_DECOMPRESS
 * @brief A decompression stream.
 */

/**
 * @struct _SquashStream
 * @extends _SquashObject
 * @brief Compression/decompression streams.
 */

/**
 * @brief Initialize a stream.
 * @protected
 *
 * @param stream The stream to initialize.
 * @param codec The codec to use.
 * @param stream_type The stream type.
 * @param options The options.
 * @param destroy_notify Function to squashl to destroy the instance.
 *
 * @see squash_object_init
 */
void
squash_stream_init (void* stream,
                    SquashCodec* codec,
                    SquashStreamType stream_type,
                    SquashOptions* options,
                    SquashDestroyNotify destroy_notify) {
  SquashStream* s;

  assert (stream != NULL);

  s = (SquashStream*) stream;

  squash_object_init (stream, false, destroy_notify);

  s->priv = NULL;

  s->next_in = NULL;
  s->avail_in = 0;
  s->total_in = 0;

  s->next_out = NULL;
  s->avail_out = 0;
  s->total_out = 0;

  s->codec = codec;
  s->options = (options != NULL) ? squash_object_ref (options) : NULL;
  s->stream_type = stream_type;
  s->state = SQUASH_STREAM_STATE_IDLE;

  s->user_data = NULL;
  s->destroy_user_data = NULL;
}

/**
 * @brief Destroy a stream.
 * @protected
 *
 * @param stream The stream.
 *
 * @see squash_object_destroy
 */
void
squash_stream_destroy (void* stream) {
  SquashStream* s;

  assert (stream != NULL);

  s = (SquashStream*) stream;

  if (s->destroy_user_data != NULL && s->user_data != NULL) {
    s->destroy_user_data (s->user_data);
  }

  if (s->options != NULL) {
    s->options = squash_object_unref (s->options);
  }

  squash_object_destroy (stream);
}

/**
 * @brief Create a new stream.
 *
 * @param codec The name of the codec.
 * @param stream_type Stream type.
 * @param ... List of key/value option pairs, followed by *NULL*
 * @return A new stream, or *NULL* on failure.
 */
SquashStream*
squash_stream_new (const char* codec,
                   SquashStreamType stream_type,
                   ...) {
  va_list options_list;
  SquashStream* stream;

  va_start (options_list, stream_type);
  stream = squash_stream_newv (codec, stream_type, options_list);
  va_end (options_list);

  return stream;
}

/**
 * @brief Create a new stream with a variadic list of options.
 *
 * @param codec The name of the codec.
 * @param stream_type Stream type.
 * @param options List of key/value option pairs, followed by *NULL*
 * @return A new stream, or *NULL* on failure.
 */
SquashStream*
squash_stream_newv (const char* codec,
                    SquashStreamType stream_type,
                    va_list options) {
  SquashOptions* opts;
  SquashCodec* codec_real;

  codec_real = squash_get_codec (codec);
  if (codec_real == NULL) {
    return NULL;
  }

  opts = squash_options_newv (codec_real, options);
  if (opts == NULL) {
    return NULL;
  }

  return squash_codec_create_stream_with_options (codec_real, stream_type, opts);
}

/**
 * @brief Create a new stream with key/value option arrays.
 *
 * @param codec The name of the codec.
 * @param stream_type Stream type.
 * @param keys *NULL*-terminated array of option keys.
 * @param values Array of option values.
 * @return A new stream, or *NULL* on failure.
 */
SquashStream*
squash_stream_newa (const char* codec,
                    SquashStreamType stream_type,
                    const char* const* keys,
                    const char* const* values) {
  return NULL;
}

/**
 * @brief Create a new stream with options.
 *
 * @param codec The name of the codec.
 * @param stream_type Stream type.
 * @param options An option group.
 * @return A new stream, or *NULL* on failure.
 */
SquashStream*
squash_stream_new_with_options (const char* codec,
                                SquashStreamType stream_type,
                                SquashOptions* options) {
  SquashCodec* codec_real;

  assert (codec != NULL);

  codec_real = squash_get_codec (codec);

  return (codec_real != NULL) ?
    squash_codec_create_stream_with_options (codec_real, stream_type, options) : NULL;
}

static SquashStatus
squash_stream_process_internal (SquashStream* stream, int operation) {
  SquashCodec* codec;
  SquashCodecFuncs* funcs = NULL;
  SquashStatus res;
  int current_operation;

  assert (stream != NULL);
  codec = stream->codec;
  assert (codec != NULL);
  funcs = squash_codec_get_funcs (codec);
  assert (funcs != NULL);

  /* Flush is optional, so return an error if it doesn't exist but
     flushing was requested. */
  if (operation == 1 && funcs->flush_stream == NULL) {
    return SQUASH_INVALID_OPERATION;
  }

  /* In order to take some of the load off of the plugins, there is
     some extra logic here which may seem a bit disorienting at first
     glance.  Basically, instead of requiring that plugins handle
     flushing or finishing with arbitrarily large inputs, we first try
     to process as much input as we can.  So, when someone calls
     squash_stream_flush or squash_stream finish Squash may, depending
     on the stream state, first call the process function.  Note that
     Squash will not flush a stream before finishing it (unless there
     is logic to do so in the plugin) as it could cause an increase in
     the output size (it does with zlib).

     One interesting consequence of this is that the stream_state
     field may not be what you're expecting.  If an earlier operation
     returned SQUASH_PROCESSING, stream_type may never transition to
     the new value.  In this case, the stream_type does accurately
     represent the state of the stream, though it probably isn't wise
     to depend on that behavior. */

  if ((operation == 0 && stream->state > SQUASH_STREAM_STATE_RUNNING) ||
      (operation == 1 && stream->state > SQUASH_STREAM_STATE_FLUSHING) ||
      (operation == 2 && stream->state > SQUASH_STREAM_STATE_FINISHING)) {
    return SQUASH_STATE;
  }

  switch (stream->state) {
    case SQUASH_STREAM_STATE_IDLE:
    case SQUASH_STREAM_STATE_RUNNING:
      current_operation = 0;
      break;
    case SQUASH_STREAM_STATE_FLUSHING:
      current_operation = 1;
      break;
    case SQUASH_STREAM_STATE_FINISHING:
      current_operation = 2;
      break;
    case SQUASH_STREAM_STATE_FINISHED:
      current_operation = 3;
      break;
  }

  if (current_operation > operation) {
    return SQUASH_STATE;
  }

  const size_t avail_in = stream->avail_in;
  const size_t avail_out = stream->avail_out;

  /* Some libraries (like zlib) will realize that we're not providing
     it any room for output and are eager to tell us that we don't
     have any space instead of decoding the stream enough to know if we
     actually need that space.

     In cases where this might be problematic, we provide a
     single-byte buffer to the plugin instead.  If anything actually
     gets written to it then we'll return an error
     (SQUASH_BUFFER_FULL), which is non-recoverable.

     There are a few cases where this might reasonably be a problem:

      * Decompression streams which know the exact size of the
        decompressed output, when using codecs which contain extra
        data at the end, such as a footer or EOS marker.

      * Compression streams writing to a fixed buffer with a length of
        less than or equal to max_compressed_size bytes.  This is a
        pretty reasonable thing to do, since you might want to only
        bother using compression if you can achieve a certain ratio.

     For consumers which don't satisfy either of these conditions,
     this code should never be reached. */

  uint8_t* next_out = NULL;
  uint8_t output_sbb = 0;
  if (stream->avail_out == 0) {
    next_out = stream->next_out;
    stream->avail_out = 1;
    stream->next_out = &output_sbb;
  }

  while (current_operation <= operation) {
    if (current_operation == 0) {
      /* Process */
      if (stream->avail_in == 0 && stream->state == SQUASH_STREAM_STATE_IDLE) {
        res = SQUASH_OK;
      } else if (funcs->process_stream != NULL) {
        res = funcs->process_stream (stream);
      } else {
        res = squash_buffer_stream_process ((SquashBufferStream*) stream);
      }
    } else if (current_operation == 1) {
      /* Flush */
      if (current_operation == operation) {
        if (funcs->flush_stream != NULL) {
          res = funcs->flush_stream (stream);
        } else {
          /* We aready checked to make sure flush_stream exists if the
             user called flush directly, so if this code is reached the
             user didn't call flush, they called finish which attempts
             to flush internally.  Just pretend it worked so we can
             proceed to invoking the finish_stream callback. */
          res = SQUASH_OK;
        }
      }
    } else if (current_operation == 2) {
      /* Finish */
      if (funcs->finish_stream != NULL) {
        res = funcs->finish_stream (stream);
      } else if (funcs->process_stream == NULL) {
        res = squash_buffer_stream_finish ((SquashBufferStream*) stream);
      } else {
        res = SQUASH_INVALID_OPERATION;
      }

      /* Plugins *should* return SQUASH_OK, not SQUASH_END_OF_STREAM,
         from the finish function, but it's an easy mistake to make
         (and correct), so... */
      if (SQUASH_UNLIKELY(res == SQUASH_END_OF_STREAM)) {
        res = SQUASH_OK;
      }
    }

    /* Check our internal single byte buffer */
    if (next_out != 0) {
      if (stream->avail_out == 0) {
        res = SQUASH_BUFFER_FULL;
      }
    }

    if (res == SQUASH_PROCESSING) {
      switch (current_operation) {
        case 0:
          stream->state = SQUASH_STREAM_STATE_RUNNING;
          break;
        case 1:
          stream->state = SQUASH_STREAM_STATE_FLUSHING;
          break;
        case 2:
          stream->state = SQUASH_STREAM_STATE_FINISHING;
          break;
      }
      break;
    } else if (res == SQUASH_END_OF_STREAM || (current_operation == 2 && res == SQUASH_OK)) {
      stream->state = SQUASH_STREAM_STATE_FINISHED;
      current_operation++;
      break;
    } else if (res == SQUASH_OK) {
      stream->state = SQUASH_STREAM_STATE_IDLE;
      current_operation++;
    } else {
      break;
    }
  }

  if (next_out != 0) {
    stream->avail_out = 0;
    stream->next_out = next_out;
  }

  stream->total_in += (avail_in - stream->avail_in);
  stream->total_out += (avail_out - stream->avail_out);

  return res;
}

/**
 * @brief Process a stream.
 *
 * This method will attempt to process data in a stream.  It should be
 * squashled repeatedly, adding data to the *avail_in* field and removing
 * data from the *avail_out* field as necessary.
 *
 * @param stream The stream.
 * @return A status code.
 * @retval SQUASH_OK All input successfully consumed.  Check the output
 *   buffer for data then proceed with new input.
 * @retval SQUASH_PROCESSING Progress was made, but not all input could
 *   be consumed.  Remove some data from the output buffer and run
 *   ::squash_stream_process again.
 * @retval SQUASH_END_OF_STREAM The end of stream was reached.  You
 *   shouldn't call ::squash_stream_process again.  *Decompression only*.
 */
SquashStatus
squash_stream_process (SquashStream* stream) {
  return squash_stream_process_internal (stream, 0);
}

/**
 * @brief Flush a stream.
 *
 * This method will attempt to process data in a stream.  It should be
 * squashled repeatedly, adding data to the *avail_in* field and removing
 * data from the *avail_out* field as necessary.
 *
 * @param stream The stream.
 * @return A status code.
 */
SquashStatus
squash_stream_flush (SquashStream* stream) {
  return squash_stream_process_internal (stream, 1);
}

/**
 * @brief Finish writing to a stream.
 *
 * @param stream The stream.
 * @return A status code.
 */
SquashStatus
squash_stream_finish (SquashStream* stream) {
  return squash_stream_process_internal (stream, 2);
}

/**
 * @}
 */
